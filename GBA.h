#ifndef _GBA_H_
#define _GBA_H_

#define ADDR_1    GPIOD
#define ADDR_2    GPIOA
#define ADDR_3    GPIOE
#define CTRLGBA   GPIOB

#define CS_SRAM    GPIO_PIN_7
#define GBA_WR     GPIO_PIN_13
#define GBA_RD     GPIO_PIN_14
#define CS_ROM     GPIO_PIN_15


void gbaScreen();

#endif