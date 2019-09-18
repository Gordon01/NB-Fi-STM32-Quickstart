#pragma once

#include "axradio.h"

#ifdef __cplusplus
extern "C" {
#endif
	
void ax5043_init(void);
void WVT_Wtimer_Init(void);
void ax5043_spi_rx(uint8_t *pData, uint16_t Size);
void ax5043_spi_tx(uint8_t *pData, uint16_t Size);
void ax5043_spi_tx_rx(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
	
	
#ifdef __cplusplus
}
#endif