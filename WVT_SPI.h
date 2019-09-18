#pragma once
#include <stm32l0xx_hal.h>

#define SPI_TIMEOUT	1000

#ifdef __cplusplus
extern "C" {
#endif

void WVT_SPI_Init(void);
void WVT_SPI_RX(uint8_t* byte, uint16_t len);
void WVT_SPI_TX(const uint8_t *byte, uint16_t len);
void WVT_SPI_RX_TX(const uint8_t *byteTx, uint8_t *byteRx, uint16_t len);
void ax5043_spi_write_cs(uint8_t state);
void WVT_MAX35103_Write_CS(uint8_t state);
uint8_t WVT_SPI_IsBusy(void);
	
#ifdef __cplusplus
}
#endif