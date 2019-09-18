#include "WVT_SPI.h"
#include "WVT_Config.h"
#include <stm32l0xx_ll_spi.h>

SPI_HandleTypeDef wvt_hspi;
uint8_t wvt_spi_busy = 0;

void WVT_SPI_Init(void)
{
	__HAL_RCC_SPI1_CLK_ENABLE();
	
	wvt_hspi.Instance = SPI1;
	wvt_hspi.Init.Mode = SPI_MODE_MASTER;
	wvt_hspi.Init.Direction = SPI_DIRECTION_2LINES;
	wvt_hspi.Init.DataSize = SPI_DATASIZE_8BIT;
	wvt_hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
	wvt_hspi.Init.NSS = SPI_NSS_SOFT;
	wvt_hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	wvt_hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	wvt_hspi.Init.TIMode = SPI_TIMODE_DISABLE;
	wvt_hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	wvt_hspi.Init.CRCPolynomial = 7;
	
	HAL_SPI_Init(&wvt_hspi);
	__HAL_SPI_ENABLE(&wvt_hspi);
	
	HAL_GPIO_WritePin(WVT_AX_CS_PORT, WVT_AX_CS_PIN, GPIO_PIN_SET);
}

void WVT_SPI_RX(uint8_t* byte, uint16_t len) 
{
	volatile uint16_t timeout;
	
	for (uint16_t i = 0; i < len; i++)
	{
		wvt_hspi.Instance->DR = 0x00;
		timeout = SPI_TIMEOUT;
		while (__HAL_SPI_GET_FLAG(&wvt_hspi, SPI_FLAG_RXNE) == RESET && timeout--) ;
		timeout = SPI_TIMEOUT;
		while (__HAL_SPI_GET_FLAG(&wvt_hspi, SPI_FLAG_BSY) == SET && timeout--) ;
		byte[i] = wvt_hspi.Instance->DR;
	}
}

void WVT_SPI_TX(const uint8_t *byte, uint16_t len) 
{
	volatile uint16_t timeout;

	for (uint16_t i = 0; i < len; i++)
	{
		wvt_hspi.Instance->DR = byte[i];
		timeout = SPI_TIMEOUT;
		while (__HAL_SPI_GET_FLAG(&wvt_hspi, SPI_FLAG_TXE) == RESET && timeout--) ;
		timeout = SPI_TIMEOUT;
		while (__HAL_SPI_GET_FLAG(&wvt_hspi, SPI_FLAG_BSY) == SET && timeout--) ;
		__HAL_SPI_CLEAR_OVRFLAG(&wvt_hspi);
	}
}

void WVT_SPI_RX_TX(const uint8_t *byteTx, uint8_t *byteRx, uint16_t len) 
{
	volatile uint16_t timeout;

	for (uint16_t i = 0; i < len; i++)
	{
		wvt_hspi.Instance->DR = byteTx[i];
		timeout = SPI_TIMEOUT;
		while (__HAL_SPI_GET_FLAG(&wvt_hspi, SPI_FLAG_RXNE) == RESET && timeout--) ;
		timeout = SPI_TIMEOUT;
		while (__HAL_SPI_GET_FLAG(&wvt_hspi, SPI_FLAG_BSY) == SET && timeout--) ;
		byteRx[i] = wvt_hspi.Instance->DR;
	}
}

void ax5043_spi_write_cs(uint8_t state)
{
	if (state)
	{
		HAL_GPIO_WritePin(WVT_AX_CS_PORT, WVT_AX_CS_PIN, GPIO_PIN_SET);
		wvt_spi_busy = 0;
	}
	else
	{
		while (wvt_spi_busy) {};
		
		wvt_spi_busy = 1;
		LL_SPI_SetClockPhase(SPI1, SPI_PHASE_1EDGE);
		HAL_GPIO_WritePin(WVT_AX_CS_PORT, WVT_AX_CS_PIN, GPIO_PIN_RESET);
	}
}

uint8_t WVT_SPI_IsBusy(void)
{
	return wvt_spi_busy;
}