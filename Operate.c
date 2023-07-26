
#include <gd32f10x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"
#include "Display.h"
#include "Operate.h"




/*********************************************************************
*
Keyboard

*/
void KeyBrdInit()
{
  //
  gpio_init(GPIOE, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, 0x3F/*GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5*/);
  gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_1);//低电量获取
}

uint8_t keyState()
{
  //
  return GPIO_ISTAT(GPIOE)&(0x3F);
}


uint8_t checkButton()
{
  uint8_t keycode = keyState();
  delay(66);

  if(gpio_input_bit_get(GPIOB,GPIO_PIN_1) == RESET)
  {
    LED_RED_ON;
  }
  else
  {
    LED_RED_OFF;
  }


  if(keyState() != keycode)
  {
    return (~keycode)&0x3F;
  }
  else
  {
    return BTNNONE;
  }
}


void WaitOKBtn()
{
  //
  while(checkButton() != BTNOK)
  {
    //delay(88);    
  }
}



// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_OLED(char * question, const char* const answers[7], int num_answers, int default_choice, uint8_t rollselect) 
{
  
  
  //clear the screen
  OledClear();

  // change the rgb led to the start menu color
  //rgbLed(default_choice);

  // print menu
  OledShowString(0,0,question,8);
  char tanswer[21] = {0};
  for (unsigned char i = 0; i < num_answers; i++) {
    memcpy(tanswer,answers[i],20);
    OledShowString(6,i+1,tanswer,8);
  }

  // start with the default choice
  unsigned char choice = default_choice;
  unsigned char choice_ori = default_choice;

  // draw selection box
  OledShowChar(0,choice,'*',8);

  //unsigned long idleTime = millis();
  uint8_t currentColor = 0;

  // wait until user makes his choice
  while (1) 
  {
    // Attract Mode
    /*
    if (millis() - idleTime > 300000) {
      if ((millis() - idleTime) % 4000 == 0) {
        if (currentColor < 7) {
          currentColor++;
          if (currentColor == 1) {
            currentColor = 2; // skip red as that signifies an error to the user
          }
        }
        else {
          currentColor = 0;
        }
      }
      rgbLed(currentColor);
    }*/
    int b = checkButton();
    if(b!=BTNNONE)
    {
      printf("getKey-%d\n",b);
    }
    
    if(b==BTNLEFT)
    {
      if(rollselect)
      {}
      else{
        //
        choice = MENU_PGUP;
        break;
      }
    }
    else
    if (b == BTNRIGHT)
    {
      if(rollselect)
      {}
      else{
        //
        choice = MENU_PGDN;
        break;
      }
    }
    else
    if (b == BTNUP)
    {
      //
      choice--;
      if(choice <= 0)
      {
        if(rollselect)
        {
          choice = num_answers;
        }
        else
        {
          //
          choice = MENU_UPUP;
          break;
        }
      }
    }
    else
    if (b == BTNDOWN)
    {
      //
      choice++;
      if(choice > num_answers)
      {
        if(rollselect)
        {
          choice = 1;
        }
        else
        {
          //
          choice = MENU_DOWNDOWN;
          break;
        }
      }
    }
    else
    if (b == BTNCANCEL)
    {
      //
      choice = MENU_CANCEL;
      break;
    }
    else 
    if (b == BTNOK) {
      //idleTime = millis();
      break;
    }
  
    //show menu selected
    if(choice != choice_ori)
    {
      //
      OledShowChar(0,choice_ori,' ',8);
      OledShowChar(0,choice,'*',8);
      choice_ori=choice;
    }




  }

  // pass on user choice
  //setColor_RGB(0, 0, 0);
  return choice;
}



uint8_t my_mkdir(char * dir)
{
  //
  uint8_t bret = false;
  bool opendir_err = 0;
  char SonPath[10][30]; //最多10层，每层最多30字符
  char RootPath[128];   //最低已存在路径
  memset(SonPath, '\0', sizeof(SonPath)); //初始化
  strcpy(RootPath, dir);
  uint8_t num = 0;           // 剔除的次数
  DIR W_Ddir;


  do                      //遍历寻找文件夹
  {
    char *dot = strrchr(RootPath, '\\');   //剔除一层
    if(dot == NULL)dot = strrchr(RootPath, '/');
    if(dot == NULL)
    {
      if(RootPath[0]){dot = RootPath;}
      else break;
    }

    strcpy(SonPath[num], dot);
    FRESULT W_Dresult = f_opendir(&W_Ddir, RootPath); //检测文件夹
    if(W_Dresult == FR_OK)
    {
            printf("Exist[%s]\r\n",RootPath);
            f_closedir(&W_Ddir);
            break;
    }
    else
    {
            printf("Err - %d[%s]\r\n",W_Dresult, RootPath);
            if(W_Dresult == FR_NO_PATH)
            {
                    opendir_err = 1;
                    num ++;
                    SonPath[num][0] = 0;
                    dot[0] = 0x00;
            }
            else return bret;
    }
  }
  while(1);


  if(opendir_err == 1)
  {
    //遍历创建文件夹
    opendir_err = 0;
    for(int i=0;i<num;i++)
    {
            strcat(RootPath, SonPath[num-i-1]);
            FRESULT W_Dresult = f_mkdir(RootPath);
            if(W_Dresult == FR_OK) { printf(">> Mk dir OK[%s]\r\n", RootPath); bret = true;}
            else printf(">> Err - %d [%s]\r\n",W_Dresult, RootPath);
    }
  }
  else bret = true;

  //
  return  bret;
}


/******************************************
  Filebrowser Module
*****************************************/
char fileNames[128][100];
char answer1[100];
char answer2[100];
char answer3[100];
char answer4[100];
char answer5[100];
char answer6[100];
char answer7[100];
char* tanswers[7] = {answer1,answer2,answer3,answer4,answer5,answer6,answer7};

void fileBrowser(char * start_dir , const char * browserTitle) 
{
  
  int currFile = 0;
  int menucnt = 0;
  // Init Dir
  strcpy(filePath,start_dir);
  // Temporary char array for filename
  char nameStr[128];
  DIR tdir;
  FRESULT fret;
  FILINFO finfo;
  bool bnomore;
  uint8_t mret;
  uint8_t default_select;


browserstart:

  // Print title
  OledClear();
  OledShowString(0,0,(char *)browserTitle,8);

  // Set currFile back to 0
  currFile = 0;
  currPage = 1;
  lastPage = 1;
  bnomore = false;

  // Open filepath directory
  if (f_opendir(&tdir,filePath) != FR_OK) {
    OledClear();
    print_Error("SD Error", true);
  }
  f_chdir(filePath);

next_page:

  menucnt = 0;
  while(1)
  {
    fret = f_readdir(&tdir,&finfo);
    if(fret == FR_OK)
    {
      //到底了或者菜单填满了
      if(finfo.fname[0] == 0x00)
      {
        //
        bnomore = true;
        break;
      }

      //if(finfo.altname[0])strcpy(fileNames[currFile],finfo.fname);else
      strcpy(fileNames[currFile],finfo.fname);
      strcpy(tanswers[menucnt],fileNames[currFile]);
      currFile++;
      menucnt++;
      printf("\nfile:[%s]-[%s]",finfo.fname,finfo.altname);

      
      if(menucnt >= 7)
      {
        //
        //lastPage++;
        break;
      }
    }
    else break;
  }

  default_select = 1;

next_page1:

  mret = questionBox_OLED((char *)browserTitle,(const char **)tanswers,menucnt,default_select,0);
  switch(mret)
  {
    case MENU_CANCEL:
    {
      for(int i = sizeof(filePath) - 1;i>0;i--)
      {
        //
        if(filePath[i] == '/'||filePath[i] == '\\')
        {
          //
          filePath[i] = 0x00;
          break;
        }
      }
      f_closedir(&tdir);
      goto browserstart;
    }
    break;
    case MENU_1:
    case MENU_2:
    case MENU_3:
    case MENU_4:
    case MENU_5:
    case MENU_6:
    case MENU_7:
    {
      //
      FIL tf;
      fret = f_open(&tf,tanswers[mret - 1],FA_OPEN_EXISTING);
      if(fret != FR_OK)
      {
        //
        strcat(filePath,"/");
        strcat(filePath,tanswers[mret - 1]);
        f_closedir(&tdir);
        goto browserstart;
      }
      else
      {
        //
        f_close(&tf);
        strcat(filePath,"/");
        strcat(filePath,tanswers[mret - 1]);
        return;
      }
    }
    break;
    case MENU_PGUP:
    case MENU_UPUP:
    {
      //
      if(currPage > 1)
      {
        currPage--;
        for(int i = 0;i<7;i++)
        {
          //
          strcpy(tanswers[i],fileNames[(currPage - 1)*7 + i]); 
        }
        bnomore = false;
        menucnt = 7;
      }
      default_select = 1;
      goto next_page1;
    }
    break;
    case MENU_PGDN:
    case MENU_DOWNDOWN:
    {
      //
      if(bnomore)
      {
        default_select = menucnt;
        goto next_page1;
      }
      currPage++;
      if(currPage > lastPage)
      {
        //
        lastPage++;
        goto next_page;
      }
      else
      {
        //
        menucnt=0;
        bnomore = false;
        for(int i=0;i<7;i++)
        {
          //
          if((currPage - 1)*7 + i < currFile)
          {
            strcpy(tanswers[i],fileNames[(currPage - 1)*7 + i]);
            menucnt++;
          }
          else
          {
            //
            bnomore = true;
            break;
          }
        }
        default_select = 1;
        goto next_page1;
      }

    }
    break;
    default:
    {
      //
      print_Error("File Err...",1);
    }
    break;
  }

  /*

  // Count files in directory
  while (myFile.openNext(&myDir, O_READ)) {
    // Ignore if hidden
    if (myFile.isHidden()) {
    }
    // Indicate a directory.
    else if (myFile.isDir()) {
      currFile++;
    }
    // It's just a file
    else if (myFile.isFile()) {
      currFile++;
    }
    myFile.close();
  }
  myDir.close();

  // "Calculate number of needed pages"
  if (currFile < 8)
    numPages = 1;
  else if (currFile < 15)
    numPages = 2;
  else if (currFile < 22)
    numPages = 3;
  else if (currFile < 29)
    numPages = 4;
  else if (currFile < 36)
    numPages = 5;

  // Fill the array "answers" with 7 options to choose from in the file browser
  char answers[7][20];

page:

  // If there are less than 7 entries, set count to that number so no empty options appear
  byte count;
  if (currFile < 8)
    count = currFile;
  else if (currPage == 1)
    count = 7;
  else if (currFile < 15)
    count = currFile - 7;
  else if (currPage == 2)
    count = 7;
  else if (currFile < 22)
    count = currFile - 14;
  else if (currPage == 3)
    count = 7;
  else if (currFile < 29)
    count = currFile - 21;
  else {
    display_Clear();
    println_Msg(F("Too many files"));
    display_Update();
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }

  // Open filepath directory
  if (!myDir.open(filePath)) {
    display_Clear();
    print_Error(F("SD Error"), true);
  }

  int countFile = 0;
  byte i = 0;
  // Cycle through all files
  while ((myFile.openNext(&myDir, O_READ)) && (i < 8)) {
    // Get name of file
    myFile.getName(nameStr, FILENAME_LENGTH);

    // Ignore if hidden
    if (myFile.isHidden()) {
    }
    // Directory
    else if (myFile.isDir()) {
      if (countFile == ((currPage - 1) * 7 + i)) {
        snprintf(fileNames[i], FILENAME_LENGTH, "%s%s", "/", nameStr);
        i++;
      }
      countFile++;
    }
    // File
    else if (myFile.isFile()) {
      if (countFile == ((currPage - 1) * 7 + i)) {
        snprintf(fileNames[i], FILENAME_LENGTH, "%s", nameStr);
        i++;
      }
      countFile++;
    }
    myFile.close();
  }
  myDir.close();

  for (byte i = 0; i < 8; i++ ) {
    // Copy short string into fileOptions
    snprintf( answers[i], FILEOPTS_LENGTH, "%s", fileNames[i] );
  }

  // Create menu with title and 1-7 options to choose from
  unsigned char answer = question_box(browserTitle, answers, count, 0);

  // Check if the page has been switched
  if (currPage != lastPage) {
    lastPage = currPage;
    goto page;
  }

  // Check if we are supposed to go back to the root dir
  if (root) {
    // Change working dir to root
    filePath[0] = '/';
    filePath[1] = '\0';
    sd.chdir("/");
    // Start again
    root = 0;
    goto browserstart;
  }

  // wait for user choice to come back from the question box menu
  switch (answer)
  {
    case 0:
      strncpy(fileName, fileNames[0], FILENAME_LENGTH - 1);
      break;

    case 1:
      strncpy(fileName, fileNames[1], FILENAME_LENGTH - 1);
      break;

    case 2:
      strncpy(fileName, fileNames[2], FILENAME_LENGTH - 1);
      break;

    case 3:
      strncpy(fileName, fileNames[3], FILENAME_LENGTH - 1);
      break;

    case 4:
      strncpy(fileName, fileNames[4], FILENAME_LENGTH - 1);
      break;

    case 5:
      strncpy(fileName, fileNames[5], FILENAME_LENGTH - 1);
      break;

    case 6:
      strncpy(fileName, fileNames[6], FILENAME_LENGTH - 1);
      break;

      //case 7:
      // File import
      //break;
  }

  // Add directory to our filepath if we just entered a new directory
  if (fileName[0] == '/') {
    // add dirname to path
    strcat(filePath, fileName);
    // Remove / from dir name
    char* dirName = fileName + 1;
    // Change working dir
    sd.chdir(dirName);
    // Start browser in new directory again
    goto browserstart;
  }
  else {
    // Afer everything is done change SD working directory back to root
    sd.chdir("/");
  }
  filebrowse = 0;

  */
}