; CartReaderApp startup file for GD32F103VC, compatible with MDK5/ARMCC.

Stack_Size      EQU     0x00000800
Heap_Size       EQU     0x00000000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

                IMPORT  SystemInit
                IMPORT  __main

__Vectors
                DCD     __initial_sp
                DCD     Reset_Handler
                DCD     NMI_Handler
                DCD     HardFault_Handler
                DCD     MemManage_Handler
                DCD     BusFault_Handler
                DCD     UsageFault_Handler
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     SVC_Handler
                DCD     DebugMon_Handler
                DCD     0
                DCD     PendSV_Handler
                DCD     SysTick_Handler

                DCD     WWDGT_IRQHandler
                DCD     LVD_IRQHandler
                DCD     TAMPER_IRQHandler
                DCD     RTC_IRQHandler
                DCD     FMC_IRQHandler
                DCD     RCU_IRQHandler
                DCD     EXTI0_IRQHandler
                DCD     EXTI1_IRQHandler
                DCD     EXTI2_IRQHandler
                DCD     EXTI3_IRQHandler
                DCD     EXTI4_IRQHandler
                DCD     DMA0_Channel0_IRQHandler
                DCD     DMA0_Channel1_IRQHandler
                DCD     DMA0_Channel2_IRQHandler
                DCD     DMA0_Channel3_IRQHandler
                DCD     DMA0_Channel4_IRQHandler
                DCD     DMA0_Channel5_IRQHandler
                DCD     DMA0_Channel6_IRQHandler
                DCD     ADC0_1_IRQHandler
                DCD     USBD_HP_CAN0_TX_IRQHandler
                DCD     USBD_LP_CAN0_RX0_IRQHandler
                DCD     CAN0_RX1_IRQHandler
                DCD     CAN0_EWMC_IRQHandler
                DCD     EXTI5_9_IRQHandler
                DCD     TIMER0_BRK_IRQHandler
                DCD     TIMER0_UP_IRQHandler
                DCD     TIMER0_TRG_CMT_IRQHandler
                DCD     TIMER0_Channel_IRQHandler
                DCD     TIMER1_IRQHandler
                DCD     TIMER2_IRQHandler
                DCD     TIMER3_IRQHandler
                DCD     I2C0_EV_IRQHandler
                DCD     I2C0_ER_IRQHandler
                DCD     I2C1_EV_IRQHandler
                DCD     I2C1_ER_IRQHandler
                DCD     SPI0_IRQHandler
                DCD     SPI1_IRQHandler
                DCD     USART0_IRQHandler
                DCD     USART1_IRQHandler
                DCD     USART2_IRQHandler
                DCD     EXTI10_15_IRQHandler
                DCD     RTC_Alarm_IRQHandler
                DCD     USBD_WKUP_IRQHandler
                DCD     TIMER7_BRK_IRQHandler
                DCD     TIMER7_UP_IRQHandler
                DCD     TIMER7_TRG_CMT_IRQHandler
                DCD     TIMER7_Channel_IRQHandler
                DCD     ADC2_IRQHandler
                DCD     EXMC_IRQHandler
                DCD     SDIO_IRQHandler
                DCD     TIMER4_IRQHandler
                DCD     SPI2_IRQHandler
                DCD     UART3_IRQHandler
                DCD     UART4_IRQHandler
                DCD     TIMER5_IRQHandler
                DCD     TIMER6_IRQHandler
                DCD     DMA1_Channel0_IRQHandler
                DCD     DMA1_Channel1_IRQHandler
                DCD     DMA1_Channel2_IRQHandler
                DCD     DMA1_Channel3_4_IRQHandler

__Vectors_End
__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                LDR     R0, =__Vectors
                LDR     R1, =0xE000ED08
                STR     R0, [R1]
                BL      SystemInit
                BL      __main
                B       .
                ENDP

Default_Handler PROC
                EXPORT  NMI_Handler               [WEAK]
                EXPORT  HardFault_Handler         [WEAK]
                EXPORT  MemManage_Handler         [WEAK]
                EXPORT  BusFault_Handler          [WEAK]
                EXPORT  UsageFault_Handler        [WEAK]
                EXPORT  SVC_Handler               [WEAK]
                EXPORT  DebugMon_Handler          [WEAK]
                EXPORT  PendSV_Handler            [WEAK]
                EXPORT  SysTick_Handler           [WEAK]
                EXPORT  WWDGT_IRQHandler          [WEAK]
                EXPORT  LVD_IRQHandler            [WEAK]
                EXPORT  TAMPER_IRQHandler         [WEAK]
                EXPORT  RTC_IRQHandler            [WEAK]
                EXPORT  FMC_IRQHandler            [WEAK]
                EXPORT  RCU_IRQHandler            [WEAK]
                EXPORT  EXTI0_IRQHandler          [WEAK]
                EXPORT  EXTI1_IRQHandler          [WEAK]
                EXPORT  EXTI2_IRQHandler          [WEAK]
                EXPORT  EXTI3_IRQHandler          [WEAK]
                EXPORT  EXTI4_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel0_IRQHandler  [WEAK]
                EXPORT  DMA0_Channel1_IRQHandler  [WEAK]
                EXPORT  DMA0_Channel2_IRQHandler  [WEAK]
                EXPORT  DMA0_Channel3_IRQHandler  [WEAK]
                EXPORT  DMA0_Channel4_IRQHandler  [WEAK]
                EXPORT  DMA0_Channel5_IRQHandler  [WEAK]
                EXPORT  DMA0_Channel6_IRQHandler  [WEAK]
                EXPORT  ADC0_1_IRQHandler         [WEAK]
                EXPORT  USBD_HP_CAN0_TX_IRQHandler [WEAK]
                EXPORT  USBD_LP_CAN0_RX0_IRQHandler [WEAK]
                EXPORT  CAN0_RX1_IRQHandler       [WEAK]
                EXPORT  CAN0_EWMC_IRQHandler      [WEAK]
                EXPORT  EXTI5_9_IRQHandler        [WEAK]
                EXPORT  TIMER0_BRK_IRQHandler     [WEAK]
                EXPORT  TIMER0_UP_IRQHandler      [WEAK]
                EXPORT  TIMER0_TRG_CMT_IRQHandler [WEAK]
                EXPORT  TIMER0_Channel_IRQHandler [WEAK]
                EXPORT  TIMER1_IRQHandler         [WEAK]
                EXPORT  TIMER2_IRQHandler         [WEAK]
                EXPORT  TIMER3_IRQHandler         [WEAK]
                EXPORT  I2C0_EV_IRQHandler        [WEAK]
                EXPORT  I2C0_ER_IRQHandler        [WEAK]
                EXPORT  I2C1_EV_IRQHandler        [WEAK]
                EXPORT  I2C1_ER_IRQHandler        [WEAK]
                EXPORT  SPI0_IRQHandler           [WEAK]
                EXPORT  SPI1_IRQHandler           [WEAK]
                EXPORT  USART0_IRQHandler         [WEAK]
                EXPORT  USART1_IRQHandler         [WEAK]
                EXPORT  USART2_IRQHandler         [WEAK]
                EXPORT  EXTI10_15_IRQHandler      [WEAK]
                EXPORT  RTC_Alarm_IRQHandler      [WEAK]
                EXPORT  USBD_WKUP_IRQHandler      [WEAK]
                EXPORT  TIMER7_BRK_IRQHandler     [WEAK]
                EXPORT  TIMER7_UP_IRQHandler      [WEAK]
                EXPORT  TIMER7_TRG_CMT_IRQHandler [WEAK]
                EXPORT  TIMER7_Channel_IRQHandler [WEAK]
                EXPORT  ADC2_IRQHandler           [WEAK]
                EXPORT  EXMC_IRQHandler           [WEAK]
                EXPORT  SDIO_IRQHandler           [WEAK]
                EXPORT  TIMER4_IRQHandler         [WEAK]
                EXPORT  SPI2_IRQHandler           [WEAK]
                EXPORT  UART3_IRQHandler          [WEAK]
                EXPORT  UART4_IRQHandler          [WEAK]
                EXPORT  TIMER5_IRQHandler         [WEAK]
                EXPORT  TIMER6_IRQHandler         [WEAK]
                EXPORT  DMA1_Channel0_IRQHandler  [WEAK]
                EXPORT  DMA1_Channel1_IRQHandler  [WEAK]
                EXPORT  DMA1_Channel2_IRQHandler  [WEAK]
                EXPORT  DMA1_Channel3_4_IRQHandler [WEAK]

NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
WWDGT_IRQHandler
LVD_IRQHandler
TAMPER_IRQHandler
RTC_IRQHandler
FMC_IRQHandler
RCU_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_IRQHandler
DMA0_Channel0_IRQHandler
DMA0_Channel1_IRQHandler
DMA0_Channel2_IRQHandler
DMA0_Channel3_IRQHandler
DMA0_Channel4_IRQHandler
DMA0_Channel5_IRQHandler
DMA0_Channel6_IRQHandler
ADC0_1_IRQHandler
USBD_HP_CAN0_TX_IRQHandler
USBD_LP_CAN0_RX0_IRQHandler
CAN0_RX1_IRQHandler
CAN0_EWMC_IRQHandler
EXTI5_9_IRQHandler
TIMER0_BRK_IRQHandler
TIMER0_UP_IRQHandler
TIMER0_TRG_CMT_IRQHandler
TIMER0_Channel_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
I2C0_EV_IRQHandler
I2C0_ER_IRQHandler
I2C1_EV_IRQHandler
I2C1_ER_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
USART0_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
EXTI10_15_IRQHandler
RTC_Alarm_IRQHandler
USBD_WKUP_IRQHandler
TIMER7_BRK_IRQHandler
TIMER7_UP_IRQHandler
TIMER7_TRG_CMT_IRQHandler
TIMER7_Channel_IRQHandler
ADC2_IRQHandler
EXMC_IRQHandler
SDIO_IRQHandler
TIMER4_IRQHandler
SPI2_IRQHandler
UART3_IRQHandler
UART4_IRQHandler
TIMER5_IRQHandler
TIMER6_IRQHandler
DMA1_Channel0_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_IRQHandler
DMA1_Channel3_4_IRQHandler
                B       .
                ENDP

                ALIGN

                IF      :DEF:__MICROLIB
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                ELSE
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap
                LDR     R0, =Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, =(Heap_Mem + Heap_Size)
                LDR     R3, =Stack_Mem
                BX      LR
                ALIGN
                ENDIF

                END