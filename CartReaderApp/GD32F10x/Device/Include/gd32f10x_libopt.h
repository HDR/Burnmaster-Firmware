/*
    Copyright (C) 2017 GigaDevice

    2017-03-13, V1.0.0, firmware for GD32F10x
*/

#ifndef GD32F10X_LIBOPT_H
#define GD32F10X_LIBOPT_H

/* Comment the line below to disable peripheral header file inclusion */
#include "RTE_Components.h"

#ifdef RTE_DEVICE_STDPERIPHERALS_ADC
#include "gd32f10x_adc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_BKP
#include "gd32f10x_bkp.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CAN
#include "gd32f10x_can.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CRC
#include "gd32f10x_crc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DAC
#include "gd32f10x_dac.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DBG
#include "gd32f10x_dbg.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DMA
#include "gd32f10x_dma.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_ENET
#include "gd32f10x_enet.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_EXMC
#include "gd32f10x_exmc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_EXTI
#include "gd32f10x_exti.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_FMC
#include "gd32f10x_fmc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_FWDGT
#include "gd32f10x_fwdgt.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_GPIO
#include "gd32f10x_gpio.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_I2C
#include "gd32f10x_i2c.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_MISC
#include "gd32f10x_misc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_PMU
#include "gd32f10x_pmu.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_RCU
#include "gd32f10x_rcu.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_RTC
#include "gd32f10x_rtc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SDIO
#include "gd32f10x_sdio.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SPI
#include "gd32f10x_spi.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_TIMER
#include "gd32f10x_timer.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_USART
#include "gd32f10x_usart.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_WWDGT
#include "gd32f10x_wwdgt.h"
#endif

#endif /* GD32F10X_LIBOPT_H */

