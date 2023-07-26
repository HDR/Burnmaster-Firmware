#include <gd32f10x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"
#include "Display.h"
#include "Operate.h"
#include "flashparam.h"
#include "fatfs/ff.h"
#include "GBM.h"
#include "GB.h"


int sramBanks;
int romBanks;
word lastByte = 0;



/******************************************
  Low level functions
*****************************************/
#define dataOut_GB() GPIO_CTL1(DATA) = 0x33333333
//inline void dataOut_GB()
//{
//  //
//  //gpio_init(DATA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(8,15));
//  GPIO_CTL1(DATA) = 0x33333333;
//}
// Switch data pins to read
#define dataIn_GB() GPIO_CTL1(DATA) = 0x44444444
//inline void dataIn_GB()
//{
//  // Set to Input
//  //gpio_init(DATA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,BITS(8,15));
//  GPIO_CTL1(DATA) = 0x44444444;
//}

void OutAddrBus(word myAddress)
{
  //
  GPIO_OCTL(ADDRLOW) = (GPIO_OCTL(ADDRLOW)&0xFFFF000F) + ((myAddress << 8) & 0xFF00) + ((myAddress >> 8) & 0xF0);
  GPIO_OCTL(ADDRHIGH) = (myAddress & 0x0F00) + (GPIO_OCTL(ADDRHIGH)&0xFFFFF0FF);
}

//#define delay_GB() __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t")

void delay_GB()
{
  //
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\tnop\n\t");

}

byte readByte_GB(word myAddress) {

  OutAddrBus(myAddress);

  //delay_GB();

  // Switch RD(PH6) to LOW
  gpio_bit_reset(CTRL,RD);

  delay_GB();

  // Read
  byte tempByte = (uint8_t)(GPIO_ISTAT(DATA) >> 8);

  // Switch and RD(PH6) to HIGH
  gpio_bit_set(CTRL,RD);

  //delay_GB();

  return tempByte;
}

void writeByte_GB(int myAddress, byte myData) 
{
  //
  OutAddrBus(myAddress);
  GPIO_OCTL(DATA) = (GPIO_OCTL(DATA)&0xFFFF00FF) + ((myData << 8) & 0xFF00);

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  delay_GB();

  // Pull WR(PH5) low
  gpio_bit_reset(CTRL,WR);

  // Leave WR low for at least 60ns
  delay_GB();

  // Pull WR(PH5) HIGH
  gpio_bit_set(CTRL,WR);

  // Leave WR high for at least 50ns
  delay_GB();
  delay_GB();
}

// Triggers CS and CLK pin
byte readByteSRAM_GB(word myAddress) {
  OutAddrBus(myAddress);

  delay_GB();

  // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
  gpio_bit_reset(CTRL,CS|CLK);
  // Pull RD(PH6) LOW
  gpio_bit_reset(CTRL,RD);

  delay_GB();

  // Read
  byte tempByte = (uint8_t)(GPIO_ISTAT(DATA) >> 8);

  // Pull RD(PH6) HIGH
  gpio_bit_set(CTRL,RD);
  if (romType == 252) {
    // Pull CS(PH3) HIGH
    gpio_bit_set(CTRL,CS);
  }
  else {
    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) HIGH
    gpio_bit_set(CTRL,CS|CLK);
  }
  delay_GB();

  return tempByte;
}

// Triggers CS and CLK pin
void writeByteSRAM_GB(int myAddress, byte myData) {
  OutAddrBus(myAddress);
  gpio_port_write(DATA,(myData << 8) & 0xFF00);

  delay_GB();

  if (romType == 252) {
    // Pull CS(PH3) LOW
    gpio_bit_reset(CTRL,CS);
    // Pull CLK(PH1)(for GB CAM) HIGH
    gpio_bit_set(CTRL,CLK);
    // Pull WR(PH5) low
    gpio_bit_reset(CTRL,WR);
  }
  else {
    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
    gpio_bit_reset(CTRL,CS|CLK);
    // Pull WR(PH5) low
    gpio_bit_reset(CTRL,WR);
  }

  // Leave WR low for at least 60ns
  delay_GB();

  if (romType == 252) {
    // Pull WR(PH5) HIGH
    gpio_bit_set(CTRL,WR);
    // Pull CS(PH3) HIGH
    gpio_bit_set(CTRL,CS);
    // Pull  CLK(PH1) LOW (for GB CAM)
    gpio_bit_reset(CTRL,CLK);
  }
  else {
    // Pull WR(PH5) HIGH
    gpio_bit_set(CTRL,WR);
    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) HIGH
    gpio_bit_set(CTRL,CS|CLK);
  }

  // Leave WR high for at least 50ns
  delay_GB();
}


/******************************************
  Game Boy functions
*****************************************/
// Read Cartridge Header
void getCartInfo_GB() 
{
  //
  romType = readByte_GB(0x0147);
  romSize = readByte_GB(0x0148);
  sramSize = readByte_GB(0x0149);

  // ROM banks
  switch (romSize) {
    case 0x00:
      romBanks = 2;
      break;
    case 0x01:
      romBanks = 4;
      break;
    case 0x02:
      romBanks = 8;
      break;
    case 0x03:
      romBanks = 16;
      break;
    case 0x04:
      romBanks = 32;
      break;
    case 0x05:
      romBanks = 64;
      break;
    case 0x06:
      romBanks = 128;
      break;
    case 0x07:
      romBanks = 256;
      break;
    default:
      romBanks = 2;
  }

  // SRAM banks
  sramBanks = 0;
  if (romType == 6) {
    sramBanks = 1;
  }

  // SRAM size
  switch (sramSize) {
    case 2:
      sramBanks = 1;
      break;
    case 3:
      sramBanks = 4;
      break;
    case 4:
      sramBanks = 16;
      break;
    case 5:
      sramBanks = 8;
      break;
  }

  // Last byte of SRAM
  if (romType == 6) {
    lastByte = 0xA1FF;
  }
  if (sramSize == 1) {
    lastByte = 0xA7FF;
  }
  else if (sramSize > 1) {
    lastByte = 0xBFFF;
  }

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", readByte_GB(0x014E), readByte_GB(0x014F));

  // Get name
  byte myByte = 0;
  byte myLength = 0;

  for (int addr = 0x0134; addr <= 0x13C; addr++) {
    myByte = readByte_GB(addr);
    if (((myByte >= 48 && myByte <= 57) || (myByte >= 65 && myByte <= 122)) && myLength < 15) {
      romName[myLength] = myByte;
      myLength++;
    }
  }
}




void showCartInfo_GB() 
{
  //
  OledClear();
  if (strcmp((const char *)checksumStr, "00") != 0) {
    OledShowString(0,0,"GB Cart Info:",8);
    OledShowString(2,1,"Name: ",8);
    OledShowString(45,1,romName,8);


    OledShowString(2,2,"Mapper: ",8);
    char * tinfo = NULL;
    if ((romType == 0) || (romType == 8) || (romType == 9))
      tinfo = "none";
    else if ((romType == 1) || (romType == 2) || (romType == 3))
      tinfo = "MBC1";
    else if ((romType == 5) || (romType == 6))
      tinfo = "MBC2";
    else if ((romType == 11) || (romType == 12) || (romType == 13))
      tinfo = "MMM01";
    else if ((romType == 15) || (romType == 16) || (romType == 17) || (romType == 18) || (romType == 19))
      tinfo = "MBC3";
    else if ((romType == 21) || (romType == 22) || (romType == 23))
      tinfo = "MBC4";
    else if ((romType == 25) || (romType == 26) || (romType == 27) || (romType == 28) || (romType == 29) || (romType == 309))
      tinfo = "MBC5";
    if (romType == 252)
      tinfo = "Camera";

    OledShowString(55,2,tinfo,8);

    OledShowString(2,3,"Rom Size: ",8);
    switch (romSize) {
      case 0:
        tinfo = "32KB";
        break;

      case 1:
        tinfo = "64KB";
        break;

      case 2:
        tinfo = "128KB";
        break;

      case 3:
        tinfo = "256KB";
        break;

      case 4:
        tinfo = "512KB";
        break;

      case 5:
        tinfo = "1MB";
        break;

      case 6:
        tinfo = "2MB";
        break;

      case 7:
        tinfo = "4MB";
        break;
    }

    OledShowString(65,3,tinfo,8);
    OledShowString(2,4,"Banks: ",8);
    char tbanks[10] = {0};
    sprintf(tbanks,"%d",romBanks);
    OledShowString(45,4,tbanks,8);

    OledShowString(2,5,"Sram Size: ",8);
    switch (sramSize) {
      case 0:
        if (romType == 6) {
          tinfo = "512B";
        }
        else {
          tinfo = "none";
        }
        break;
      case 1:
        tinfo = "2KB";
        break;

      case 2:
        tinfo = "8KB";
        break;

      case 3:
        tinfo = "32KB";
        break;

      case 4:
        tinfo = "128KB";
        break;

      default: tinfo = "none";
    }
    OledShowString(70,5,tinfo,8);

    OledShowString(2,6,"Checksum: ",8);
    OledShowString(65,6,checksumStr,8);

    // Wait for user input
    OledShowString(0,7,"Press Button...",8);
    WaitOKBtn();
  }
  else {
    OledShowString(0,2,"GAMEPAK ERROR",8);
  }
}


/******************************************
   Setup
 *****************************************/
void setup_GB() {
  
  // Set Address Pins to Output
  



  //A0-A7(D8-D15),A12-A15(D4-D7)
  gpio_init(ADDRLOW,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(4,15));
  gpio_port_write(ADDRLOW,0xFFFF);

  //A8-A11
  gpio_init(ADDRHIGH,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11);
  gpio_bit_set(ADDRHIGH,BITS(8,11));
  


  // Set Control Pins to Output RST(B3) CLK(B12) CS(B15) WR(B13) RD(B14)  
  gpio_init(CTRL,GPIO_MODE_OUT_PP,GPIO_OSPEED_2MHZ,RST|CS|WR|RD|CLK);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  gpio_bit_reset(CTRL,RST);
  delay(1);
  gpio_bit_set(CTRL,RST|CS|WR|RD);
  // Output a low signal on CLK to disable writing GB Camera RAM
  gpio_bit_reset(CTRL,CLK);

  // Set Data Pins (D0-D7) to Input
  gpio_init(DATA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,BITS(8,15));
  // Disable Internal Pullups
  //PORTC = 0x00;

  delay(100);

  // Print start page

  getCartInfo_GB();
  showCartInfo_GB();
}



/******************************************
  ROM functions
*****************************************/
// Read ROM
void readROM_GB() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // create a new folder for the rom file
  load_dword(foldern);
  f_chdir("/");
  sprintf(folder, "GB/ROM/%s/%d", romName, foldern);

  FRESULT rst;
  FIL tfile;

  rst = my_mkdir(folder);
  rst = f_chdir(folder);

  OledClear();
  OledShowString(0,0,"Saving to ",8);
  OledShowString(4,1,folder,8);
  //printf("/..."));

  // write new folder number back to eeprom
  foldern = foldern + 1;
  save_dword(foldern);

  //open file on sd card
  rst = f_open(&tfile,fileName, FA_CREATE_ALWAYS|FA_WRITE);
  if (rst != FR_OK) {
    print_Error("Can't create file", 1);
  }

  word romAddress = 0;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(romBanks) * 16384;
  draw_progressbar(0, totalProgressBar,3);

  for (word currBank = 1; currBank < romBanks; currBank++) {
    // Switch data pins to output
    dataOut_GB();

    LED_GREEN_BLINK;

    // Set ROM bank for MBC2/3/4/5
    if (romType >= 5) {
      writeByte_GB(0x2100, currBank);
    }
    // Set ROM bank for MBC1
    else {
      writeByte_GB(0x6000, 0);
      writeByte_GB(0x4000, currBank >> 5);
      writeByte_GB(0x2000, currBank & 0x1F);
    }

    // Switch data pins to intput
    dataIn_GB();

    // Second bank starts at 0x4000
    if (currBank > 1) {
      romAddress = 0x4000;
    }

    
    // Read banks and save to SD
    while (romAddress <= 0x7FFF) {
      for (int i = 0; i < 512; i++) {
        sdBuffer[i] = readByte_GB(romAddress + i);
      }
      UINT dwt = 0;
      rst = f_write(&tfile,sdBuffer, 512,&dwt);
      romAddress += 512;
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar,3);
    }
  }

  // Close the file:
  f_close(&tfile);
}

// Calculate checksum
uint16_t calc_checksum_GB (char* fileName, char* folder) {
  uint16_t calcChecksum = 0;
  //  int calcFilesize = 0; // unused
  unsigned long i = 0;
  int c = 0;
  FIL tfile;

  if (strcmp(folder, "root") != 0)
    f_chdir(folder);

  // If file exists
  //printf("\r\nChecksum for file : %s",fileName);
  if (f_open(&tfile,fileName, FA_READ) == FR_OK) {
    //calcFilesize = myFile.fileSize() * 8 / 1024 / 1024; // unused
    for (i = 0; i < (f_size(&tfile) / 512); i++) {
      UINT rdt = 0;
      f_read(&tfile,sdBuffer, 512,&rdt);
      for (c = 0; c < 512; c++) {
        calcChecksum += sdBuffer[c];
      }
    }
    f_close(&tfile);
    //f_chdir();
    // Subtract checksum bytes
    printf("\r\nCheckSum : %04x",calcChecksum);

    byte b1 = readByte_GB(0x014E);
    byte b2 = readByte_GB(0x014F);
    //printf("\r\nb1=%02x,b2=%02x",b1,b2);
    calcChecksum -= b1;
    calcChecksum -= b2;

    // Return result
    return (calcChecksum);
  }
  // Else show error
  else {
    print_Error("DUMP ROM 1ST", false);
    return 0;
  }
}

// Compare checksum
boolean compare_checksum_GB() {
  OledShowString(0,3,"Calculating Checksum",8);

  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // last used rom folder
  load_dword(foldern);
  sprintf(folder, "GB/ROM/%s/%d", romName, foldern - 1);

  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum_GB(fileName, folder));

  if (strcmp(calcsumStr, checksumStr) == 0) {
    OledShowString(0,4,"Result: ",8);
    OledShowString(50,4,calcsumStr,8);
    OledShowString(0,5,"Checksum matches",8);
    return 1;
  }
  else {
    OledShowString(0,4,"Result: ",8);
    OledShowString(50,4,calcsumStr,8);
    print_Error("Checksum Error", false);
    return 0;
  }
}


/******************************************
  SRAM functions
*****************************************/
// Read RAM
void readSRAM_GB() {
  // Does cartridge have RAM
  if (lastByte > 0) {

    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".sav");

    // create a new folder for the save file
    load_dword(foldern);
    sprintf(folder, "GB/SAVE/%s/%d", romName, foldern);
    my_mkdir(folder);
    f_chdir(folder);

    // write new folder number back to eeprom
    foldern = foldern + 1;
    save_dword(foldern);

    //open file on sd card
    FIL tfile;
    if (f_open(&tfile, fileName, FA_CREATE_ALWAYS|FA_WRITE) != FR_OK) {
      print_Error("SD Error", true);
    }

    dataIn_GB();

    // MBC2 Fix
    readByte_GB(0x0134);

    dataOut_GB();
    if (romType <= 4) {
      writeByte_GB(0x6000, 1);
    }

    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch SRAM banks
    for (byte currBank = 0; currBank < sramBanks; currBank++) {
      dataOut_GB();
      writeByte_GB(0x4000, currBank);

      // Read SRAM
      dataIn_GB();
      for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress += 64) {
        for (byte i = 0; i < 64; i++) {
          sdBuffer[i] = readByteSRAM_GB(sramAddress + i);
        }
        UINT wrt;
        f_write(&tfile, sdBuffer, 64, &wrt);
      }
    }

    // Disable SRAM
    dataOut_GB();
    writeByte_GB(0x0000, 0x00);
    dataIn_GB();

    // Close the file:
    f_close(&tfile);

    // Signal end of process
    OledShowString(0,0,"Saved to ",8);
    OledShowString(4,1,folder,8);
    //printf("/"));
  }
  else {
    print_Error("Cart has no SRAM", false);
  }
}

// Write RAM
void writeSRAM_GB() {
  // Does cartridge have SRAM
  if (lastByte > 0) {
    // Create filepath
    //sprintf(filePath, "%s/%s", filePath, fileName);

    //open file on sd card
    FIL tfile;
    if (f_open(&tfile, filePath, FA_READ) == FR_OK) {
      // Set pins to input
      dataIn_GB();

      // MBC2 Fix
      readByte_GB(0x0134);

      dataOut_GB();

      // Enable SRAM for MBC1
      if (romType <= 4) {
        writeByte_GB(0x6000, 1);
      }

      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch RAM banks
      for (byte currBank = 0; currBank < sramBanks; currBank++) {
        writeByte_GB(0x4000, currBank);

        // Write RAM
        for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress++) {
          byte bdata;
          UINT rdt = 0;
          f_read(&tfile,&bdata,1,&rdt);
          writeByteSRAM_GB(sramAddress, bdata);
        }
      }
      // Disable SRAM
      writeByte_GB(0x0000, 0x00);

      // Set pins to input
      dataIn_GB();

      // Close the file:
      f_close(&tfile);
      OledClear();
      OledShowString(0,2,"SRAM writing finished",8);

    }
    else {
      print_Error("File doesnt exist", false);
    }
  }
  else {
    print_Error("Cart has no SRAM", false);
  }
}

// Check if the SRAM was written without any error
unsigned long verifySRAM_GB() {

  //open file on sd card
  FIL tfile;
  if (f_open(&tfile,filePath, FA_READ) == FR_OK) {

    // Variable for errors
    writeErrors = 0;

    dataIn_GB();

    // MBC2 Fix
    readByte_GB(0x0134);

    // Check SRAM size
    if (lastByte > 0) {
      dataOut_GB();
      if (romType <= 4) { // MBC1
        writeByte_GB(0x6000, 1); // Set RAM Mode
      }

      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch SRAM banks
      for (byte currBank = 0; currBank < sramBanks; currBank++) {
        dataOut_GB();
        writeByte_GB(0x4000, currBank);

        // Read SRAM
        dataIn_GB();
        for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress += 64) {
          //fill sdBuffer
          UINT rdt;
          f_read(&tfile, sdBuffer, 64, &rdt);
          for (int c = 0; c < 64; c++) {
            if (readByteSRAM_GB(sramAddress + c) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
      dataOut_GB();
      // Disable RAM
      writeByte_GB(0x0000, 0x00);
      dataIn_GB();
    }
    // Close the file:
    f_close(&tfile);
    return writeErrors;
  }
  else {
    print_Error("Can't open file", true);
  }
  return 0;
}




/******************************************
  29F016/29F032/29F033 flashrom functions
*****************************************/
// Write 29F032 flashrom
// A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
// A14-A21 connected to MBC5
void writeFlash29F_GB(byte MBC, boolean flashErase) {
  // Launch filebrowser
  filePath[0] = '\0';
  f_chdir("/");
  fileBrowser("/","Select file:");
  OledClear();

  FIL tf;
  UINT rdt;
  uint16_t wfid;
  char msgbuf[64] = {0};

  // Open file on sd card
  if (f_open(&tf,filePath, FA_READ) == FR_OK) 
  {
    // Get rom size from file
    f_lseek(&tf,0x147);
    f_read(&tf,&romType,1,&rdt);
    f_read(&tf,&romSize,1,&rdt);
    // Go back to file beginning
    f_lseek(&tf,0);

    // ROM banks
    if(romSize < 8)
      romBanks = 1 << (romSize + 1);
    else 
      romBanks = 2;

    // Set data pins to output
    dataOut_GB();

    // Set ROM bank hi 0
    writeByte_GB(0x3000, 0);
    // Set ROM bank low 0
    writeByte_GB(0x2000, 0);
    delay(100);

    // Reset flash
    writeByte_GB(0x555, 0xf0);
    delay(100);

    // ID command sequence
    writeByte_GB(0x555, 0xaa);
    delay(1);
    writeByte_GB(0x2aa, 0x55);
    delay(1);
    writeByte_GB(0x555, 0x90);
    delay(1);

    dataIn_GB();

    // Read the two id bytes into a string
    wfid = readByte_GB(0);
    wfid = (wfid << 8)&0xFF00;
    wfid += readByte_GB(1)&0xFF;
    sprintf(flashid, "%04X", wfid);

    if (wfid == 0x04d4) {
      sprintf(msgbuf,"MBM29F033C\nBanks: %d/256",romBanks);
    }
    else if (wfid == 0x0141) {
      sprintf(msgbuf,"AM29F032B\nBanks: %d/256",romBanks);
    }
    else if (wfid == 0x01AD) {
      sprintf(msgbuf,"AM29F016B\nBanks: %d/256",romBanks);
    }
    else if (wfid == 0x04AD) {
      sprintf(msgbuf,"AM29F016D\nBanks: %d/256",romBanks);
    }
    else if (wfid == 0x01D5) {
      sprintf(msgbuf,"AM29F080B\nBanks: %d/256",romBanks);
    }
    else {
      
      OledShowString(0,0,"Flash ID: ",8);
      OledShowString(60,0,flashid,8);
      f_close(&tf);
      print_Error("Unknown flashrom", true);
    }

    //
    OledShowString(0,0,msgbuf,8);

    dataOut_GB();

    // Reset flash
    writeByte_GB(0x555, 0xf0);
    delay(100);

    if (flashErase) 
    {
      OledShowString(0,3,"Erasing flash...",8);
      //display_Update();

      // Erase flash
      writeByte_GB(0x555, 0xaa);
      writeByte_GB(0x2aa, 0x55);
      writeByte_GB(0x555, 0x80);
      writeByte_GB(0x555, 0xaa);
      writeByte_GB(0x2aa, 0x55);
      writeByte_GB(0x555, 0x10);

      // Set data pins to input
      dataIn_GB();
      // Read the status register
      byte statusReg = readByte_GB(0);
      // After a completed erase D7 will output 1
      while ((statusReg & 0x80) != 0x80) {
        // Update Status
        statusReg = readByte_GB(0);
      }

      // Blankcheck
      OledShowString(0,4,"Blank check...",8);

      // Read x number of banks
      for (int currBank = 0; currBank < romBanks; currBank++) 
      {
        // Blink led
        LED_GREEN_BLINK;

        dataOut_GB();

        // Set ROM bank
        writeByte_GB(0x2000, currBank);
        dataIn_GB();

        for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
          for (int currByte = 0; currByte < 512; currByte++) {
            sdBuffer[currByte] = readByte_GB(currAddr + currByte);
          }
          for (int j = 0; j < 512; j++) {
            if (sdBuffer[j] != 0xFF) {
              OledShowString(0,5,"Not empty",8);
              f_close(&tf);
              print_Error("Erase failed!", true);
            }
          }
        }
      }
    }



    if (MBC == 3) {
      OledShowString(0,5,"Writing flash MBC3",8);

      // Write flash
      dataOut_GB();

      word currAddr = 0;
      word endAddr = 0x3FFF;

      //Initialize progress bar
      uint32_t processedProgressBar = 0;
      uint32_t totalProgressBar = (uint32_t)(romBanks) * 16384;
      draw_progressbar(0, totalProgressBar,6);

      for (int currBank = 0; currBank < romBanks; currBank++) {
        // Blink led
        LED_GREEN_BLINK;

        // Set ROM bank
        writeByte_GB(0x2100, currBank);

        if (currBank > 0) {
          currAddr = 0x4000;
          endAddr = 0x7FFF;
        }

        while (currAddr <= endAddr) {
          f_read(&tf,sdBuffer, 512,&rdt);
          for (int currByte = 0; currByte < 512; currByte++) 
          {
            // Write command sequence
            writeByte_GB(0x555, 0xaa);
            writeByte_GB(0x2aa, 0x55);
            writeByte_GB(0x555, 0xa0);
            // Write current byte
            writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

            // Set data pins to input
            dataIn_GB();

            // Set OE/RD(PH6) LOW
            gpio_bit_reset(CTRL,RD);
            //PORTH &= ~(1 << 6);

            // Busy check
            while (((GPIO_ISTAT(DATA) >> 8) & 0x80) != (sdBuffer[currByte] & 0x80)) {
            }

            // Switch OE/RD(PH6) to HIGH
            gpio_bit_set(CTRL,RD);
            //PORTH |= (1 << 6);

            // Set data pins to output
            dataOut_GB();
          }
          currAddr += 512;
          processedProgressBar += 512;
          draw_progressbar(processedProgressBar, totalProgressBar,6);
        }
      }
    }

    else if (MBC == 5) 
    {
      OledShowString(0,5,"Writing flash MBC5",8);

      // Write flash
      dataOut_GB();

      //Initialize progress bar
      uint32_t processedProgressBar = 0;
      uint32_t totalProgressBar = (uint32_t)(romBanks) * 16384;
      draw_progressbar(0, totalProgressBar, 6);

      for (int currBank = 0; currBank < romBanks; currBank++) {
        // Blink led
        LED_GREEN_BLINK;

        // Set ROM bank
        writeByte_GB(0x2000, currBank);
        // 0x2A8000 fix
        writeByte_GB(0x4000, 0x0);

        for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) 
        {
          f_read(&tf,sdBuffer, 512,&rdt);

          for (int currByte = 0; currByte < 512; currByte++) {
            // Write command sequence
            writeByte_GB(0x555, 0xaa);
            writeByte_GB(0x2aa, 0x55);
            writeByte_GB(0x555, 0xa0);
            // Write current byte
            writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

            // Set data pins to input
            dataIn_GB();

            // Set OE/RD(PH6) LOW
            gpio_bit_reset(CTRL,RD);

            // Busy check
            while (((GPIO_ISTAT(DATA) >> 8) & 0x80) != (sdBuffer[currByte] & 0x80)) {

            }

            // Switch OE/RD(PH6) to HIGH
            gpio_bit_set(CTRL,RD);

            // Set data pins to output
            dataOut_GB();
          }
          processedProgressBar += 512;
          draw_progressbar(processedProgressBar, totalProgressBar, 6);
        }
      }
    }

    // Set data pins to input again
    dataIn_GB();

    OledClear();
    OledShowString(0,0,"Verifying...",8);

    // Go back to file beginning
    f_lseek(&tf,0);
    //unsigned int addr = 0;  // unused
    writeErrors = 0;
    // Verify flashrom
    word romAddress = 0;

    // Read number of banks and switch banks
    for (word bank = 1; bank < romBanks; bank++) {
      // Switch data pins to output
      dataOut_GB();

      if (romType >= 5) { // MBC2 and above
        writeByte_GB(0x2100, bank); // Set ROM bank
      }
      else { // MBC1
        writeByte_GB(0x6000, 0); // Set ROM Mode
        writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
      }

      // Switch data pins to intput
      dataIn_GB();

      if (bank > 1) {
        romAddress = 0x4000;
      }
      // Blink led
      LED_GREEN_BLINK;

      // Read up to 7FFF per bank
      while (romAddress <= 0x7FFF) {
        // Fill sdBuffer
        f_read(&tf,sdBuffer, 512,&rdt);
        // Compare
        for (int i = 0; i < 512; i++) {
          if (readByte_GB(romAddress + i) != sdBuffer[i]) {
            writeErrors++;
          }
        }
        romAddress += 512;
      }
    }
    // Close the file:
    f_close(&tf);

    if (writeErrors == 0) {
      OledShowString(0,2,"OK!",8);
      //display_Update();
    }
    else {
      sprintf(msgbuf,"Error %d bytes...",writeErrors);
      OledShowString(0,2,msgbuf,8);
      print_Error("Did not verify :(", true);
    }
  }
  else {
    OledShowString(0,0,"Can't open file!",8);
  }
}




/******************************************
  CFU flashrom functions
*****************************************/

bool flashX16Mode;
bool flashSwitchLastBits;
unsigned long flashBanks;

/*
   Flash chips can either be in x8 mode or x16 mode and sometimes the two
   least significant bits on flash cartridges' data lines are swapped.
   This function reads a byte and compensates for the differences.
   This is only necessary for commands to the flash, not for data read from the flash, the MBC or SRAM.

   address needs to be the x8 mode address of the flash register that should be read.
*/
byte readByteCompensated(int address) {
  byte data = readByte_GB(address >> (flashX16Mode ? 1 : 0));
  if (flashSwitchLastBits) {
    return (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
  }
  return data;
}

/*
   Flash chips can either be in x8 mode or x16 mode and sometimes the two
   least significant bits on flash cartridges' data lines are swapped.
   This function writes a byte and compensates for the differences.
   This is only necessary for commands to the flash, not for data written to the flash, the MBC or SRAM.
   .
   address needs to be the x8 mode address of the flash register that should be read.
*/
void writeByteCompensated(int address, byte data) {
  byte td;
  if (flashSwitchLastBits) {
    td = (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
  }
  else td = data;
  writeByte_GB(address >> (flashX16Mode ? 1 : 0), td);
}

void startCFIMode(boolean x16Mode) {
  if (x16Mode) {
    writeByte_GB(0x555, 0xf0); //x16 mode reset command
    delay(500);
    writeByte_GB(0x555, 0xf0); //Double reset to get out of possible Autoselect + CFI mode
    delay(500);
    writeByte_GB(0x55, 0x98);  //x16 CFI Query command
  } else {
    writeByte_GB(0xAAA, 0xf0); //x8  mode reset command
    delay(100);
    writeByte_GB(0xAAA, 0xf0); //Double reset to get out of possible Autoselect + CFI mode
    delay(100);
    writeByte_GB(0xAA, 0x98);  //x8 CFI Query command
  }
}

/* Identify the different flash chips.
   Sets the global variables flashBanks, flashX16Mode and flashSwitchLastBits
*/
void identifyCFI_GB() 
{
  // Reset flash
  OledClear();
  dataOut_GB();
  writeByte_GB(0x6000, 0); // Set ROM Mode
  writeByte_GB(0x2000, 0); // Set Bank to 0
  writeByte_GB(0x3000, 0);

  startCFIMode(false); // Trying x8 mode first

  dataIn_GB();
  OledClear();
  // Try x8 mode first
  char cfiQRYx8[7];
  char cfiQRYx16[7];
  sprintf(cfiQRYx8, "%02X%02X%02X", readByte_GB(0x20), readByte_GB(0x22), readByte_GB(0x24));
  sprintf(cfiQRYx16, "%02X%02X%02X", readByte_GB(0x10), readByte_GB(0x11), readByte_GB(0x12)); // some devices use x8-style CFI Query command even though they are in x16 command mode
  if (strcmp(cfiQRYx8, "515259") == 0) 
  { // QRY in x8 mode
    printf("Normal CFI x8 Mode");
    flashX16Mode = false;
    flashSwitchLastBits = false;
  } 
  else if (strcmp(cfiQRYx8, "52515A") == 0) 
  { // QRY in x8 mode with switched last bit
    printf("Switched CFI x8 Mode");
    flashX16Mode = false;
    flashSwitchLastBits = true;
  } 
  else if (strcmp(cfiQRYx16, "515259") == 0) 
  { // QRY in x16 mode
    printf("Normal CFI x16 Mode");
    flashX16Mode = true;
    flashSwitchLastBits = false;
  } 
  else if (strcmp(cfiQRYx16, "52515A") == 0) 
  { // QRY in x16 mode with switched last bit
    printf("Switched CFI x16 Mode");
    flashX16Mode = true;
    flashSwitchLastBits = true;
  } 
  else 
  {
    startCFIMode(true); // Try x16 mode next
    sprintf(cfiQRYx16, "%02X%02X%02X", readByte_GB(0x10), readByte_GB(0x11), readByte_GB(0x12));
    if (strcmp(cfiQRYx16, "515259") == 0) { // QRY in x16 mode
      printf("Normal CFI x16 Mode");
      flashX16Mode = true;
      flashSwitchLastBits = false;
    } else if (strcmp(cfiQRYx16, "52515A") == 0) { // QRY in x16 mode with switched last bit
      printf("Switched CFI x16 Mode");
      flashX16Mode = true;
      flashSwitchLastBits = true;
    } else {
      printf("CFI Query failed!");
      WaitOKBtn();
      return;
    }
  }
  dataIn_GB();
  flashBanks = 1 << (readByteCompensated(0x4E) - 14); // - flashX16Mode);
  dataOut_GB();

  // Reset flash
  writeByteCompensated(0xAAA, 0xf0);
  delay(100);
}

// Write 29F032 flashrom
// A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
// A14-A21 connected to MBC5
// identifyFlash_GB() needs to be run before this!

bool writeCFI_GB() {
  //
  FIL tf;
  UINT rdt;
  char msgbuf[128] = {0};
  uint32_t use_tick = getSystick();
  // Open file on sd card
  if (f_open(&tf,filePath, FA_READ) == FR_OK) 
  {
    // Get rom size from file
    f_lseek(&tf,0x147);
    f_read(&tf,&romType,1,&rdt);
    f_read(&tf,&romSize,1,&rdt);
    // Go back to file beginning
    f_lseek(&tf,0);

    // ROM banks
    if(romSize < 8)
      romBanks = 1 << (romSize + 1);
    else 
      romBanks = 2;



    if (romBanks <= flashBanks) 
    {
      //
      sprintf(msgbuf,"Using %d/%d Banks",romBanks,flashBanks);
      OledShowString(0,0,msgbuf,8);
    } 
    else 
    {
      sprintf(msgbuf,"Error:\nFlash has too few banks!\nHas %d\nbut needs %d banks.",flashBanks,romBanks);
      OledShowString(0,0,msgbuf,8);
      OledShowString(0,7,"Press OK button...",8);
      f_close(&tf);
      WaitOKBtn();
      ResetSystem();
    }

    // Set data pins to output
    dataOut_GB();

    // Set ROM bank hi 0
    writeByte_GB(0x3000, 0);
    // Set ROM bank low 0
    writeByte_GB(0x2000, 0);
    delay(100);

    // Reset flash
    writeByteCompensated(0xAAA, 0xf0);
    delay(100);
    dataOut_GB();

    // Reset flash
    writeByte_GB(0x555, 0xf0);

    delay(100);


    // Erase flash   
    OledShowString(0,1,"Erasing...",8);



    int lastSector = (romBanks << 1);
    printf("lastSector=%d\n",lastSector);
    for (int currSector = 0x0; currSector < lastSector; currSector++)
    {

      int SA = ((currSector >> 1)?0x4000:0) + (currSector & 0x01)*0x2000;
      dataOut_GB();
      //writeByte_GB(0x2000, 0);
      writeByte_GB(0x2100, currSector >> 1);
      delayMicroseconds(1); 
      
            
      writeByteCompensated(0xAAA, 0xAA);
      //delayMicroseconds(1);   
      writeByteCompensated(0x555, 0x55);
      //delayMicroseconds(1);   
      writeByteCompensated(0xAAA, 0x80);
      //delayMicroseconds(1);   
      writeByteCompensated(0xAAA, 0xAA);
      //delayMicroseconds(1);   
      writeByteCompensated(0x555, 0x55);

      //writeByte_GB(0x2000, currSector >> 1);
      //delayMicroseconds(1); 
      writeByteCompensated(SA, 0x30);
      //delay(50);

      // Blink LED
      LED_BLUE_BLINK;
      showPersent(currSector,lastSector,61,1);

      //
      dataIn_GB();
      // Read the status register
      byte statusReg = readByte_GB(SA);
      printf("curSector = %d,SA=0x%04x\n",currSector,SA);

      // After a completed erase D7 will output 1
      while ((statusReg | 0x7F) != 0xFF) {
        // Blink led
        delay(5);
        // Update Status
        statusReg = readByte_GB(SA);
      }

      
    }

    showPersent(1,1,61,1);


    /*
    writeByteCompensated(0xAAA, 0xaa);
    writeByteCompensated(0x555, 0x55);
    writeByteCompensated(0xAAA, 0x80);
    writeByteCompensated(0xAAA, 0xaa);
    writeByteCompensated(0x555, 0x55);
    writeByteCompensated(0xAAA, 0x10);

    dataIn_GB();

    // Read the status register
    byte statusReg = readByte_GB(0);

    // After a completed erase D7 will output 1
    while ((statusReg & 0x80) != 0x80) {
      // Blink led
      LED_GREEN_BLINK;
      delay(100);
      // Update Status
      statusReg = readByte_GB(0);
    }
*/

    
    // Blankcheck
    OledShowString(0,2,"Blank check...",8);
    // Read x number of banks
    for (int currBank = 0; currBank < romBanks; currBank++) 
    {
      // Blink led
      LED_GREEN_BLINK;
      showPersent(currBank,romBanks,84,2);
      dataOut_GB();

      // Set ROM bank
      writeByte_GB(0x2100, currBank);
      dataIn_GB();

      uint16_t addrfrom = currBank > 0?0x4000:0;
      uint16_t addrto = currBank > 0?0x7FFF:0x3FFF;
      for (unsigned int currAddr = addrfrom; currAddr < addrto; currAddr += 512) {
        for (int currByte = 0; currByte < 512; currByte++) {
          sdBuffer[currByte] = readByte_GB(currAddr + currByte);
        }
        for (int j = 0; j < 512; j++) {
          if (sdBuffer[j] != 0xFF) {
            OledShowString(0,3,"Not empty",8);
            f_close(&tf);
            print_Error("Erase failed", true);
          }
        }
      }
    }
    showPersent(1,1,84,2);

 


    OledShowString(0,3,"Writing flash MBC3/5",8);

    // Write flash
    // Set data pins to output
    dataOut_GB();

    // Set ROM bank hi 0
    writeByte_GB(0x3000, 0);
    // Set ROM bank low 0
    writeByte_GB(0x2000, 0);
    delay(100);

    // Reset flash
    writeByteCompensated(0xAAA, 0xf0);
    delay(100);
    dataOut_GB();

    // Reset flash
    writeByte_GB(0x555, 0xf0);

    delay(100);



    word currAddr = 0;
    word endAddr = 0x3FFF;

    for (int currBank = 0; currBank < romBanks; currBank++) 
    {
      // Blink led
      LED_GREEN_BLINK;
      showPersent(currBank,romBanks,10,4);

      // Set ROM bank
      writeByte_GB(0x2100, currBank);
      writeByte_GB(0x3000, 0x0);//bank addr high byte, maybe recovered by normal write, need to reset to zero here.

      if (currBank > 0) 
      {
        currAddr = 0x4000;
        endAddr = 0x7FFF;
      }
      //else
      {
        // 0x2A8000 fix
        
      }

      while (currAddr <= endAddr)
      {
        f_read(&tf,sdBuffer, 512,&rdt);

        for (int currByte = 0; currByte < 512; currByte++) 
        {
          // Write command sequence
          writeByteCompensated(0xAAA, 0xaa);
          writeByteCompensated(0x555, 0x55);
          writeByteCompensated(0xAAA, 0xa0);

          // Write current byte
          writeByteCompensated(currAddr + currByte, sdBuffer[currByte]);

          delay_GB();
          // Set data pins to input
          dataIn_GB();

                    delay_GB();
          // Setting CS(PH3) and OE/RD(PH6) LOW
          //PORTH &= ~((1 << 3) | (1 << 6));
          gpio_bit_reset(CTRL,CS);

          //delay_GB();
          gpio_bit_reset(CTRL,RD);
          //delay_GB();

          // Busy check
          short i = 0;

          //for(i = 0;i<40;i++){delay_GB();}          
          while (((GPIO_ISTAT(DATA) >> 8) & 0x80) != (sdBuffer[currByte] & 0x80)) 
          {
            i++;
            if (i > 2000) 
            {
              if (currAddr >= 0x4000) 
              { 
                // This happens when trying to flash an MBC5 as if it was an MBC3. Retry to flash as MBC5, starting from last successfull byte.
                currByte--;
                currAddr += 0x4000;
                endAddr = 0x7FFF;
                break;
              } 
              else 
              { 
                // If a timeout happens while trying to flash MBC5-style, flashing failed.
                f_close(&tf);
                return false;
              }
            }
          }

          // Switch CS(PH3) and OE/RD(PH6) to HIGH
          gpio_bit_set(CTRL,RD);
          gpio_bit_set(CTRL,CS);
          
          // Waste a few CPU cycles to remove write errors
          delay_GB();
          delay_GB();
          delay_GB();

          // Set data pins to output
          dataOut_GB();
        }
        currAddr += 512;
      }
    }
    showPersent(1,1,10,4);

    // Set data pins to input again
    dataIn_GB();

    //OledClear();
    OledShowString(0,5,"Verifying...",8);

    // Go back to file beginning
    f_lseek(&tf,0);
    //unsigned int addr = 0;  // unused
    writeErrors = 0;

    // Verify flashrom
    word romAddress = 0;

    // Read number of banks and switch banks
    for (word bank = 1; bank < romBanks; bank++) 
    {
      // Switch data pins to output
      dataOut_GB();

      if (romType >= 5) { // MBC2 and above
        writeByte_GB(0x2100, bank); // Set ROM bank
      }
      else { // MBC1
        writeByte_GB(0x6000, 0); // Set ROM Mode
        writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
      }

      // Switch data pins to intput
      dataIn_GB();

      if (bank > 1) {
        romAddress = 0x4000;
      }
      // Blink led
      LED_GREEN_BLINK;
      showPersent(bank - 1,romBanks,72,5);

      // Read up to 7FFF per bank
      while (romAddress <= 0x7FFF) {
        // Fill sdBuffer
        f_read(&tf, sdBuffer, 512, &rdt);
        // Compare
        for (int i = 0; i < 512; i++) {
          if (readByte_GB(romAddress + i) != sdBuffer[i]) {
            writeErrors++;
          }
        }
        romAddress += 512;
      }
    }

    showPersent(1,1,72,5);
    // Close the file:
    f_close(&tf);

    if (writeErrors == 0) {
      //OledShowString(0,6,"OK",8);
      use_tick = (getSystick() - use_tick)/1000;
      sprintf(msgbuf,"Use Time: %d(s)",use_tick);
      OledShowString(10,6,msgbuf,8);
    }
    else {
      sprintf(msgbuf,"Error:%d bytes",writeErrors);
      OledShowString(0,6,msgbuf,8);
      print_Error("Did not verify...", false);
    }
  }
  else {
    OledShowString(0,1,"Can't open file!",8);
  }
  return true;
}


// GB Flash items
static const char GBFlashItem1[] = "Flash Cart";
static const char GBFlashItem2[] = "Flash Cart and Save";
static const char GBFlashItem3[] = "29F Cart (MBC3)";
static const char GBFlashItem4[] = "29F Cart (MBC5)";
static const char GBFlashItem5[] = "29F Cart (CAM)";

//static const char GBFlashItem6[] = "GB Smart";
static const char GBFlashItem7[] = "Reset";
static const char* const menuOptionsGBFlash[] = {GBFlashItem1, GBFlashItem2, GBFlashItem3, GBFlashItem4, GBFlashItem5, GBFlashItem7};


uint8_t gbFlashMenu()
{
  //
  uint8_t bret = 0;

  unsigned char gbFlash = questionBox_OLED("Select type:", menuOptionsGBFlash, 6, 1, 1);
  OledClear();
  // wait for user choice to come back from the question box menu
  switch (gbFlash)
  {
    case 0:
      //cancel btn clicked
      bret = 1;
      break;
 

    case 1:
      // Flash CFI
      // Launch filebrowser
      fileBrowser("/","Select file:");
      OledClear();
      identifyCFI_GB();
      if (!writeCFI_GB()) {
        OledClear();
        OledShowString(0,0,"Flashing failed\nTime out!",8);
      }
      break;

    case 2:
      // Flash CFI and Save
      fileBrowser("/","Select file:");
      OledClear();
      identifyCFI_GB();
      if (!writeCFI_GB()) {
        //
        print_Error("Flashing failed!\n Time out!",true);
      }
      getCartInfo_GB();
      // Does cartridge have SRAM
      if (lastByte > 0) 
      {
        //
        OledClear();
        OledShowString(0,0,"Save Sram Data:",8);
        //Get the save file name
        char * cpos = strrchr(filePath,'/');
        if(cpos){cpos++;strcpy(fileName,cpos);}
        else strcpy(fileName,filePath);
        //Remove file ext name
        int pos = -1;
        while (fileName[++pos] != '\0') {
          if (fileName[pos] == '.') {
            fileName[pos] = '\0';
            break;
          }
        }


        sprintf(filePath, "/GB/SAVE/%s/", fileName);
        bool saveFound = false;
        FILINFO tfinfo;
        if (f_stat(filePath,&tfinfo) == FR_OK) 
        {
          foldern = load_dword();
          for (int i = foldern; i >= 0; i--) 
          {
            sprintf(filePath, "/GB/SAVE/%s/%d/%s.SAV", fileName, i, fileName);
            if (f_stat(filePath,&tfinfo) == FR_OK) 
            {
              //
              char tmsg[64] = {0};
              sprintf(tmsg,"Save number %d found.",i);
              OledShowString(0,1,tmsg,8);
              saveFound = true;

              writeSRAM_GB();

              unsigned long wrErrors = verifySRAM_GB();
              if (wrErrors == 0) 
              {
                OledShowString(0,2,"Verified OK",8);
              }
              else 
              {
                sprintf(tmsg,"Error: %d bytes.",wrErrors);
                OledShowString(0,2,tmsg,8);
                print_Error("Did not verify...", false);
              }
              break;
            }
          }
        }
        
        if (!saveFound) 
        {
          OledShowString(0,1,"Error: No save found.",8);
        }
      }
      else 
      {
        print_Error("Cart has no Sram", false);
      }
      break;

   case 3:
      //Flash MBC3
      writeFlash29F_GB(3, 1);
      // Reset
      break;

   case 4:
      //Flash MBC5
      writeFlash29F_GB(5, 1);
      break;
   case 5:
      //Flash GB Camera
      //MBC3
      writeFlash29F_GB(3, 1);
      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();

      OledClear();
      OledShowString(0,0,"Please change the",8);
      OledShowString(0,1,"switch on the cart",8);
      OledShowString(0,2,"to B2 (Bank 2)",8);
      OledShowString(0,3,"if you want to flash",8);
      OledShowString(0,4,"a second game",8);

      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();

      // Flash second bank without erase
      // Change working dir to root
      //MBC3
      writeFlash29F_GB(3, 0);
      break;

      /*
    case 6:
      // Flash GB Smart
      setup_GBSmart();
      mode = mode_GB_GBSmart;
      break;*/

    case 6:
      ResetSystem();
      break;
  }

  if(bret == 0)
  {
    // Reset
    OledShowString(0,7,"Press OK Button...",8);
    WaitOKBtn();
    ResetSystem();
  }
  return bret;
}


void gbFlashScreen()
{
  while(1)
  {
    //
    setup_GB();
    uint8_t b = gbFlashMenu();
    if(b>0)break;
  }
}

// GB menu items
static const char GBMenuItem1[] = "Flash GBC Cart";
static const char GBMenuItem2[] = "Read Rom";
static const char GBMenuItem3[] = "Read Save";
static const char GBMenuItem4[] = "Write Save";
static const char GBMenuItem5[] = "NPower GB Memory";
static const char GBMenuItem6[] = "Reset";
static const char* const menuOptionsGB[] = {GBMenuItem1, GBMenuItem2, GBMenuItem3, GBMenuItem4, GBMenuItem5, GBMenuItem6};

uint8_t gbMenu() 
{
  //
  uint8_t bret = 0;
  
  // create menu with title and 3 options to choose from
  unsigned char gbMenu = questionBox_OLED("GB Cart Reader", menuOptionsGB, 6, 1, 1);

  // wait for user choice to come back from the question box menu
  switch (gbMenu)
  {
    case 0:
      //cancel btn clicked
      bret = 1;
      break;
    case 1:
      gbFlashScreen();
      break;
    case 2:
      OledClear();
      // Change working dir to root
      //f_chdir("/");
      readROM_GB();
      compare_checksum_GB();
      break;

    case 3:
      OledClear();
      // Does cartridge have SRAM
      if (lastByte > 0) {
      // Change working dir to root
        f_chdir("/");
        readSRAM_GB();
      }
      else {
        print_Error("Cart has no Sram", false);
      }
      break;

    case 4:
      OledClear();
      // Does cartridge have SRAM
      if (lastByte > 0) 
      {
        // Change working dir to root
        f_chdir("/");
        filePath[0] = '\0';
        fileBrowser("/","Select sav file");
        writeSRAM_GB();
        OledClear();
        unsigned long wrErrors;
        wrErrors = verifySRAM_GB();
        if (wrErrors == 0) 
        {
          OledShowString(0,2,"Verified OK",8);
        }
        else 
        {
          char tbufp[30] = {0};
          sprintf(tbufp,"Error: %d bytes.",wrErrors);
          OledShowString(0,1,tbufp,8);
          print_Error("did not verify.", false);
        }
      }
      else {
        print_Error("Cart has no Sram", false);
      }
      break;

    case 5:
      // Flash GB Memory
      gbmScreen();
      break;
    case 6:
      ResetSystem();
      break;
  }

  //OledClear();
  if(bret == 0)
  {
    OledShowString(0,7,"Press OK Button...",8);
    WaitOKBtn();
  }
  return bret;
}


void gbScreen()
{
  while(1)
  {
    //
    setup_GB();
    uint8_t b = gbMenu();
    if(b>0)break;
  }
}




//******************************************
// End of File
//******************************************