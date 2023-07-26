#pragma onece

#define BTNNONE (0)
#define BTNUP (1)
#define BTNDOWN (2)
#define BTNLEFT (4)
#define BTNRIGHT (8)
#define BTNCANCEL (16)
#define BTNOK (32)


#define MENU_CANCEL (0)
#define MENU_1 (1)
#define MENU_2 (2)
#define MENU_3 (3)
#define MENU_4 (4)
#define MENU_5 (5)
#define MENU_6 (6)
#define MENU_7 (7)
#define MENU_UPUP (100)
#define MENU_DOWNDOWN (101)
#define MENU_PGUP (102)
#define MENU_PGDN (103)


void KeyBrdInit();
uint8_t keyState();
uint8_t checkButton();
void WaitOKBtn();

// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_OLED(char * question, const char* const answers[7], int num_answers, int default_choice, uint8_t rollselect);
uint8_t my_mkdir(char * dir);
void fileBrowser(char * start_dir , const char * browserTitle);