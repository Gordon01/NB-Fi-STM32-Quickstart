#pragma once

// Red LED
#define WVT_RED_LED_PIN			GPIO_PIN_7
#define WVT_RED_LED_PORT		GPIOB

// AX5043
#define WVT_AX_IRQ_PIN			GPIO_PIN_3
#define WVT_AX_IRQ_PORT			GPIOA
#define WVT_AX_IRQn				EXTI2_3_IRQn
#define	WVT_AX_IRQ_HANDLER(x)	EXTI2_3_IRQHandler(x)

#define WVT_AX_CS_PIN			GPIO_PIN_4
#define WVT_AX_CS_PORT	        GPIOA

#define WVT_AX_POW_PIN 			GPIO_PIN_2
#define WVT_AX_POW_PORT 		GPIOA

#define WVT_AX_MISO_PIN 		GPIO_PIN_6
#define WVT_AX_MISO_PORT 		GPIOA

#define WVT_AX_MOSI_PIN 		GPIO_PIN_7
#define WVT_AX_MOSI_PORT 		GPIOA

#define WVT_AX_SCK_PIN 			GPIO_PIN_5
#define WVT_AX_SCK_PORT 		GPIOA


#define EEPROM_INT_nbfi_data			(DATA_EEPROM_BASE + 1024*5)
/** @brief	ID модема и ключ шифрования сохранены во флеш-памяти
 **			MODEM_ID = FLASH_BASE + 130944
 **			KEY      = FLASH_BASE + 130948 */
#define MODEM_ID  *((const uint32_t*)0x0801ff80UL)  
#define KEY		   ((const uint32_t*)0x0801ff84UL)      