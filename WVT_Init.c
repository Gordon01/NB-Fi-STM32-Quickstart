#include <stm32l0xx_hal.h>
#include <stm32l0xx_ll_rcc.h>
#include <string.h>
#include "WVT_Config.h"
#include "WVT_Radio.h"
#include "nbfi.h"
#include "nbfi_config.h"
#include "WVT_SPI.h"

void WVT_GPIO_Init_Pin(GPIO_TypeDef *port, uint32_t pin, uint32_t mode, uint32_t pull)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = pin;
	GPIO_InitStructure.Mode = mode;
	// Наименьшая скорость нарастания выходного напряжения
	// меньше расход и наводки
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Pull = pull;
	
	HAL_GPIO_Init(port, &GPIO_InitStructure);
}

void WVT_GPIO_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	// Red LED
	WVT_GPIO_Init_Pin(WVT_RED_LED_PORT, WVT_RED_LED_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
	
	// AX5043
	WVT_GPIO_Init_Pin(WVT_AX_IRQ_PORT, WVT_AX_IRQ_PIN, GPIO_MODE_IT_RISING, GPIO_PULLDOWN);
	WVT_GPIO_Init_Pin(WVT_AX_CS_PORT, WVT_AX_CS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
	WVT_GPIO_Init_Pin(WVT_AX_POW_PORT, WVT_AX_POW_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
	WVT_GPIO_Init_Pin(WVT_AX_MISO_PORT, WVT_AX_MISO_PIN, GPIO_MODE_AF_PP, GPIO_PULLDOWN);
	WVT_GPIO_Init_Pin(WVT_AX_MOSI_PORT, WVT_AX_MOSI_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL);
	WVT_GPIO_Init_Pin(WVT_AX_SCK_PORT, WVT_AX_SCK_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL);
	
	// Выключает питание радио, если оно не инициализируется
	HAL_GPIO_WritePin(WVT_AX_POW_PORT, WVT_AX_POW_PIN, GPIO_PIN_SET);
}

void WVT_Clock_Init_Error_Handler()
{
	// Если выполнение остананавливается на этой точке останова, то какая-то 
	// частота не инициализирована. Проверь стек вызовов.
	asm("bkpt 255");
}

void SystemClock_Config(void)
{
	/* По какой-то неведомой причине порядок объявления структур влияет на
	 * корректную инициализацию SPI (!). Так что не меняй его без надобности
	 **/
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
	
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	HAL_PWR_EnableBkUpAccess();
	
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI | RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		WVT_Clock_Init_Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		WVT_Clock_Init_Error_Handler();
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_LPTIM1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_LSI;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		WVT_Clock_Init_Error_Handler();
	}
	
	LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_MSI);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void WVT_NVIC_Init(void)
{
	// Lowest priority to prevent interference with other ISR's
	HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);
	
	HAL_NVIC_SetPriority(WVT_AX_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(WVT_AX_IRQn);
	
	HAL_NVIC_SetPriority(LPTIM1_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
}

void WVT_Init(void)
{
	HAL_Init();
	
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	
	if (0)
	{
		__HAL_RCC_DBGMCU_CLK_ENABLE();
		HAL_DBGMCU_EnableDBGStopMode();
		__HAL_DBGMCU_FREEZE_LPTIMER();
		__HAL_DBGMCU_FREEZE_RTC();
		__HAL_DBGMCU_FREEZE_IWDG();		
	}
	else
	{
		__HAL_RCC_DBGMCU_CLK_DISABLE();
	}
	
	SystemClock_Config();
	WVT_GPIO_Init();
	WVT_SPI_Init();
	WVT_Wtimer_Init();
	ax5043_init();
	WVT_NVIC_Init();
}