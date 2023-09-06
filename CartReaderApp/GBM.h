#ifndef _GBM_H_
#define _GBM_H_

#define ADDRLOW    GPIOD
#define ADDRHIGH   GPIOA
#define DATA       GPIOE
#define CTRL       GPIOB

#define RST        GPIO_PIN_7
#define AUDIO_IN   GPIO_PIN_8
#define CLK        GPIO_PIN_12
#define WR         GPIO_PIN_13
#define RD         GPIO_PIN_14
#define CS         GPIO_PIN_15

void gbmScreen();


#endif