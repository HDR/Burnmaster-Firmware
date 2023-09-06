#pragma onece

#define I2C1_SLAVE_ADDRESS7    0x78
#define SSD1306_ADDR 0x3c
#define MAX_COLUMN 128

#define LED1 (1)
#define LED_B (2)
#define LED_G (4)
#define LED_R (8)



void I2cInit(void);
void SSD1306_WriteCmd(uint8_t var);
void SSD1306_WriteData(uint8_t var);



//坐标设置：也就是在哪里显示
void OledSetPos(uint8_t x, uint8_t y);
//开启Oled显示
void OledDisplayOn(void);
//关闭Oled显示   
void OledDisplayOff(void);
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样  
void OledClear(void);
//在指定位置显示一个字符,包括部分字符
//x:0~127，y:0~7
//Char_Size:选择字体 16/12 
void OledShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
//显示一个字符串
uint8_t OledShowString(uint8_t x,uint8_t y,char *str,uint8_t Char_Size);
//显示一个位图
void OledShowPicData(uint8_t x,uint8_t y,uint8_t wdt,uint8_t hgt,uint8_t *pPicData);
//初始化
void OledInit(void);



void setColor_RGB(uint8_t r, uint8_t g, uint8_t b);
void print_Error(char *errorMessage, uint8_t forceReset);
void draw_progressbar(uint32_t processed, uint32_t total, uint8_t line);
void showPersent(uint32_t processed, uint32_t total, uint8_t x, uint8_t line);




//Leds
void LEDSInit();
void LED_ON(uint8_t LedNum);
void LED_OFF(uint8_t LedNum);
void LED_BLINK(uint8_t LedNum);
void LED_CLEAR(void);

#define LED_RED_ON LED_ON(LED_R)
#define LED_GREEN_ON LED_ON(LED_G)
#define LED_BLUE_ON LED_ON(LED_B)
#define LED_RED_OFF LED_OFF(LED_R)
#define LED_GREEN_OFF LED_OFF(LED_G)
#define LED_BLUE_OFF LED_OFF(LED_B)
#define LED_RED_BLINK LED_BLINK(LED_R)
#define LED_GREEN_BLINK LED_BLINK(LED_G)
#define LED_BLUE_BLINK LED_BLINK(LED_B)