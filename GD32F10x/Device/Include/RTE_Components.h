/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : RTE_Components.h
Purpose : Header file to set defines for include files
*/

#ifndef RTE_COMPONENTS_H            // Avoid multiple inclusion.
#define RTE_COMPONENTS_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define RTE_DEVICE_STDPERIPHERALS_ADC
#define RTE_DEVICE_STDPERIPHERALS_CAN
#define RTE_DEVICE_STDPERIPHERALS_CEC
#define RTE_DEVICE_STDPERIPHERALS_CRC
#define RTE_DEVICE_STDPERIPHERALS_CMP
#define RTE_DEVICE_STDPERIPHERALS_DAC
#define RTE_DEVICE_STDPERIPHERALS_DBG
#define RTE_DEVICE_STDPERIPHERALS_DMA
#define RTE_DEVICE_STDPERIPHERALS_EXTI
#define RTE_DEVICE_STDPERIPHERALS_FMC
#define RTE_DEVICE_STDPERIPHERALS_GPIO
#define RTE_DEVICE_STDPERIPHERALS_SYSCFG
#define RTE_DEVICE_STDPERIPHERALS_I2C
#define RTE_DEVICE_STDPERIPHERALS_FWDGT
#define RTE_DEVICE_STDPERIPHERALS_PMU
#define RTE_DEVICE_STDPERIPHERALS_RCU
#define RTE_DEVICE_STDPERIPHERALS_RTC
#define RTE_DEVICE_STDPERIPHERALS_SPI
#define RTE_DEVICE_STDPERIPHERALS_TIMER
#define RTE_DEVICE_STDPERIPHERALS_USART
#define RTE_DEVICE_STDPERIPHERALS_WWDGT
#define RTE_DEVICE_STDPERIPHERALS_MISC
#define RTE_DEVICE_STDPERIPHERALS_TSI
#define RTE_DEVICE_STDPERIPHERALS_SLCD
#define RTE_DEVICE_STDPERIPHERALS_OPA
#define RTE_DEVICE_STDPERIPHERALS_IVREF

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/

