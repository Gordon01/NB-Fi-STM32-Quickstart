#include "ax5043.h"
#include "axradio.h"
#include "WVT_Radio.h"
#include "wtimer.h"
#include "string.h"
#include "nbfi.h"
#include "rf.h"
#include "nbfi_config.h"
#include "nbfi_defines.h"
#include "WVT_SPI.h"
#include "WVT_Config.h"
#include <stm32l0xx_hal.h>

LPTIM_HandleTypeDef wvt_hlptim;

void WVT_Radio_Callback(uint8_t * data, uint16_t length);
uint32_t WVT_ADC_Get_Temperature(void);
uint32_t WVT_ADC_Get_Voltage(void);

#if BAND_ID == UL868800_DL446000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND_ID == UL868800_DL864000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       864000000
#elif BAND_ID == UL868800_DL446000_DL864000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       864000000
#elif BAND_ID == UL867950_DL446000
#define NBFI_UL_FREQ_BASE       (867950000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND_ID == UL868500_DL446000
#define NBFI_UL_FREQ_BASE       (868500000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND_ID == UL868100_DL446000
#define NBFI_UL_FREQ_BASE       (868100000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND_ID == UL864000_DL446000
#define NBFI_UL_FREQ_BASE       (864000000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND_ID == UL863175_DL446000
#define NBFI_UL_FREQ_BASE       (863175000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND_ID == UL864000_DL875000
#define NBFI_UL_FREQ_BASE       (864000000 - 25000)
#define NBFI_DL_FREQ_BASE       875000000
#elif BAND_ID == UL868800_DL869100
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       869100000
#elif BAND_ID == UL866975_DL865000
#define NBFI_UL_FREQ_BASE       (866975000 - 25000)
#define NBFI_DL_FREQ_BASE       865000000
#endif 

const nbfi_settings_t nbfi_set_default =
{
    DRX,				//mode;
    UL_DBPSK_50_PROT_D, // tx_phy_channel;
    DL_PSK_200,         // rx_phy_channel;
    HANDSHAKE_SIMPLE,
    MACK_16,            //mack_mode
    2,                  //num_of_retries;
    8,                  //max_payload_len;
    {0},                //dl_ID[3];
    {0},                //temp_ID[3];
    {0xFF,0,0},         //broadcast_ID[3];
    {0},                //full_ID[6];
    0,                  //tx_freq;
    0,                  //rx_freq;
    PCB,                //tx_antenna;
    PCB,                //rx_antenna;
    RF_MAX_POWER,       //tx_pwr;
    3600*6,             //heartbeat_interval
    255,                //heartbeat_num
    0,                  //additional_flags
    NBFI_UL_FREQ_BASE,
    NBFI_DL_FREQ_BASE
};


void ax5043_enable_global_irq(void)
{
	__enable_irq();
}

void ax5043_disable_global_irq(void)
{
	__disable_irq();
}

void ax5043_enable_pin_irq(void)
{
	HAL_NVIC_EnableIRQ(WVT_AX_IRQn);
}

void ax5043_disable_pin_irq(void)
{
	HAL_NVIC_DisableIRQ(WVT_AX_IRQn);
}

uint8_t ax5043_get_irq_pin_state(void)
{
	return HAL_GPIO_ReadPin(WVT_AX_IRQ_PORT, WVT_AX_IRQ_PIN);
}

inline void ax5043_spi_rx(uint8_t *pData, uint16_t Size)
{
	WVT_SPI_RX(pData, Size);
}

inline void ax5043_spi_tx(uint8_t *pData, uint16_t Size)
{
	WVT_SPI_TX(pData, Size);
}

inline void ax5043_spi_tx_rx(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
	WVT_SPI_RX_TX(pTxData, pRxData, Size);
}

void ax5043_on_off_pwr(uint8_t pwr)
{
	//ax5043 asic external power on/off implementation might be here for hardware reset  
	if(pwr)
	{
		// Есть ли утечка через этот пин? хммм, надо проверить
		//HAL_GPIO_WritePin(WVT_AX_CS_PORT, WVT_AX_CS_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(WVT_AX_POW_PORT, WVT_AX_POW_PIN, GPIO_PIN_RESET);
	}
	else 
	{
		// Есть ли утечка через этот пин? хммм
		//HAL_GPIO_WritePin(WVT_AX_CS_PORT, WVT_AX_CS_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(WVT_AX_POW_PORT, WVT_AX_POW_PIN, GPIO_PIN_SET);
	}
	HAL_Delay(10);
}

void wtimer_cc_irq_enable(uint8_t chan)
{
	__HAL_LPTIM_ENABLE_IT(&wvt_hlptim, LPTIM_IT_CMPM);
}

void wtimer_cc_irq_disable(uint8_t chan)
{
	__HAL_LPTIM_DISABLE_IT(&wvt_hlptim, LPTIM_IT_CMPM);
}

void wtimer_cc_set(uint8_t chan, uint16_t data)
{
	wvt_hlptim.Instance->CMP = data;
}

uint16_t wtimer_cc_get(uint8_t chan)
{
	return (uint16_t) wvt_hlptim.Instance->CMP;
}

uint16_t wtimer_cnt_get(uint8_t chan)
{
	static uint16_t prev = 0; 
	uint16_t timer = (uint16_t) wvt_hlptim.Instance->CNT;
	if ((timer < prev) && ((prev - timer) < 10000))
	{
		return prev;
	}
	prev = timer;
	return timer;
}

uint8_t wtimer_check_cc_irq(uint8_t chan)
{
	return __HAL_LPTIM_GET_FLAG(&wvt_hlptim, LPTIM_IT_CMPM);
}

void nbfi_read_default_settings(nbfi_settings_t* settings)
{
	for (uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
	{
		((uint8_t *)settings)[i] = ((uint8_t *)&nbfi_set_default)[i];
	}
}

void nbfi_read_flash_settings(nbfi_settings_t* settings) 
{
	memcpy((void*)settings, ((const void*)EEPROM_INT_nbfi_data), sizeof(nbfi_settings_t));
}

void nbfi_write_flash_settings(nbfi_settings_t* settings)
{
	if (HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) return;
	for (uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
	{
		if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_INT_nbfi_data + i, ((uint8_t *)settings)[i]) != HAL_OK) break;
	}
	HAL_FLASHEx_DATAEEPROM_Lock(); 
}

uint32_t nbfi_measure_valtage_or_temperature(uint8_t val)
{
	if (val == 0)
	{
		return WVT_ADC_Get_Temperature();
	}
	else
	{
		return WVT_ADC_Get_Voltage();
	}
}

uint32_t nbfi_update_rtc()
{
	//you should use this callback when external RTC used
	//return rtc_counter;  
	return 0;
}

void nbfi_rtc_synchronized(uint32_t time)
{
	//you should use this callback for RTC counter correction when external RTC used
	//rtc_counter = time;
  
}

uint8_t nbfi_lock = 1;

void nbfi_lock_unlock_nbfi_irq(uint8_t lock)
{
	nbfi_lock = lock;
}

void WVT_Radio_Callback(uint8_t * data, uint16_t length)
{
	// This is called when modem receives downlink
}

uint32_t WVT_ADC_Get_Temperature(void)
{
	return 228;
}

uint32_t WVT_ADC_Get_Voltage(void)
{
	return 330;
}

void RADIO_LPTIM_Init(void)
{
	__HAL_RCC_LPTIM1_CLK_ENABLE();

	wvt_hlptim.Instance = LPTIM1;
	wvt_hlptim.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	wvt_hlptim.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
	wvt_hlptim.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	wvt_hlptim.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
	wvt_hlptim.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	wvt_hlptim.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  
	HAL_LPTIM_Init(&wvt_hlptim);
}

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
	wtimer_cc0_irq();
}

void WVT_Wtimer_Init(void)
{
	RADIO_LPTIM_Init();       
        
	HAL_LPTIM_Counter_Start(&wvt_hlptim, 0xffff);
                      
	ax5043_reg_func(AXRADIO_ENABLE_GLOBAL_IRQ, (void*)ax5043_enable_global_irq);
	ax5043_reg_func(AXRADIO_DISABLE_GLOBAL_IRQ, (void*)ax5043_disable_global_irq);
	ax5043_reg_func(AXRADIO_ENABLE_IRQ_PIN, (void*)ax5043_enable_pin_irq);
	ax5043_reg_func(AXRADIO_DISABLE_IRQ_PIN, (void*)ax5043_disable_pin_irq);
	ax5043_reg_func(AXRADIO_GET_IRQ_PIN, (void*)ax5043_get_irq_pin_state);
	ax5043_reg_func(AXRADIO_SPI_RX, (void*)ax5043_spi_rx);
	ax5043_reg_func(AXRADIO_SPI_TX, (void*)ax5043_spi_tx);
	ax5043_reg_func(AXRADIO_SPI_TX_RX, (void*)ax5043_spi_tx_rx);
	ax5043_reg_func(AXRADIO_SPI_CS_WRITE, (void*)ax5043_spi_write_cs);
	ax5043_reg_func(AXRADIO_ON_OFF_PWR, (void*)ax5043_on_off_pwr);

	wtimer_reg_func(WTIMER_GLOBAL_IRQ_ENABLE, (void*)ax5043_enable_global_irq);
	wtimer_reg_func(WTIMER_GLOBAL_IRQ_DISABLE, (void*)ax5043_disable_global_irq);
	wtimer_reg_func(WTIMER_CC_IRQ_ENABLE, (void*)wtimer_cc_irq_enable);
	wtimer_reg_func(WTIMER_CC_IRQ_DISABLE, (void*)wtimer_cc_irq_disable);
	wtimer_reg_func(WTIMER_SET_CC, (void*)wtimer_cc_set);
	wtimer_reg_func(WTIMER_GET_CC, (void*)wtimer_cc_get);
	wtimer_reg_func(WTIMER_GET_CNT, (void*)wtimer_cnt_get);
	wtimer_reg_func(WTIMER_CHECK_CC_IRQ, (void*)wtimer_check_cc_irq);

	wtimer_init();
}

void ax5043_init(void)
{
	nbfi_lock = 0;
        
	/*
	NBFI_reg_func(NBFI_BEFORE_TX, (void*)nbfi_before_tx);
	NBFI_reg_func(NBFI_BEFORE_RX, (void*)nbfi_before_rx);
        NBFI_reg_func(NBFI_BEFORE_OFF, (void*)nbfi_before_off);
	*/
	NBFI_reg_func(NBFI_RECEIVE_COMLETE, (void*)WVT_Radio_Callback);
	NBFI_reg_func(NBFI_READ_FLASH_SETTINGS, (void*)nbfi_read_flash_settings);
	NBFI_reg_func(NBFI_WRITE_FLASH_SETTINGS, (void*)nbfi_write_flash_settings);
	NBFI_reg_func(NBFI_READ_DEFAULT_SETTINGS, (void*)nbfi_read_default_settings);
	NBFI_reg_func(NBFI_MEASURE_VOLTAGE_OR_TEMPERATURE, (void*)nbfi_measure_valtage_or_temperature);
        
	//register callbacks when external RTC used
	//NBFI_reg_func(NBFI_UPDATE_RTC, (void*)nbfi_update_rtc);
	//NBFI_reg_func(NBFI_RTC_SYNCHRONIZED, (void*)nbfi_rtc_synchronized);
        
	nbfi_dev_info_t info = {
		MODEM_ID,
		(uint32_t*)KEY,
		RF_MIN_POWER,
		RF_MAX_POWER, 
		HARDWARE_ID,
		HARDWARE_REV,
		BAND_ID,
		SEND_INFO_INTERVAL
	};

	NBFi_Config_Set_Device_Info(&info);
	//NBFi_Clear_Saved_Configuration(); //if you need to clear previously saved nbfi configuration in EEPROM
	NBFI_Init(0);
}


