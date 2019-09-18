#include <stm32l0xx_hal.h>
#include "wtimer.h"
#include "nbfi.h"
#include "easyax5043.h"
#include "WVT_Init.h"
#include "WVT_Config.h"

extern LPTIM_HandleTypeDef wvt_hlptim;

#ifdef __cplusplus
extern "C" {
#endif
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void WVT_AX_IRQ_HANDLER(void)
{
	axradio_isr();
	HAL_GPIO_EXTI_IRQHandler(WVT_AX_IRQ_PIN);
}

void LPTIM1_IRQHandler(void)
{
	HAL_LPTIM_IRQHandler(&wvt_hlptim);
}	
	
#ifdef __cplusplus
}
#endif

int main(void)
{
	WVT_Init();

	while (1)
	{
		wtimer_runcallbacks();
							
		if (axradio_cansleep() && NBFi_can_sleep() && wtimer_cansleep()) 
		{
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		}
	}
}
