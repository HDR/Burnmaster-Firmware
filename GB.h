#pragma onece

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


extern int sramBanks;
extern int romBanks;
extern word lastByte;

void TestMemGB(boolean bFast);
void gbFlashScreen();
void gbScreen();