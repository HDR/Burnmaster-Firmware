#include <gd32f10x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"
#include "Display.h"
#include "Operate.h"
#include "flashparam.h"
#include "fatfs/ff.h"
#include "GBA.h"




/******************************************
   Variables
 *****************************************/
char calcChecksumStr[5];
boolean readType;
unsigned long cartSize;
char cartID[5];
byte romVersion = 0;





/******************************************
   Low level functions
*****************************************/
void delay_GBA()
{
  //__asm__("nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}



void setROM_GBA() 
{
  // CS_SRAM(B3)
  // CS_ROM(B15)
  // WR(B13)
  // RD(B14)
  gpio_init(CTRLGBA,GPIO_MODE_OUT_PP,GPIO_OSPEED_2MHZ,CS_SRAM|CS_ROM|GBA_WR|GBA_RD);
  gpio_bit_set(CTRLGBA,CS_SRAM|CS_ROM|GBA_WR|GBA_RD);
  // AD0-AD7
  gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(4,15));
  // AD8-AD15
  gpio_init(ADDR_2,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(8,11));
  // AD16-AD23
  gpio_init(ADDR_3,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(8,15));
  // Wait
  delay(688);
}



//#define TEST_MY_CART

word readWord_GBA(unsigned long Address) 
{


  // Divide address by two to get word addressing
  unsigned long myAddress = Address >> 1;


  // Set address/data ports to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;
  //gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,BITS(4,15));
  //gpio_init(ADDR_2,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,BITS(8,11));
  //gpio_init(ADDR_3,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,BITS(8,15));


  // Output address to address pins,
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);
  GPIO_OCTL(ADDR_3) = (GPIO_OCTL(ADDR_3)&0xFFFF00FF) + ((myAddress >> 8)&0xFF00);

  // Pull CS(PH3) to LOW
  gpio_bit_reset(CTRLGBA,CS_ROM);

  delay_GBA();
  delay_GBA();

  #ifdef TEST_MY_CART
  //delayMicroseconds(1);
  #endif

  // Set address/data ports to input
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF);
  GPIO_CTL1(ADDR_1) = 0x44444444;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x44440000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x4444;//A8-A11

  //gpio_init(ADDR_1,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,BITS(4,15));
  //gpio_init(ADDR_2,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,BITS(8,11));
  //gpio_init(ADDR_3,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,BITS(8,15));

    
  //delay_GBA(); 

  // Pull RD(PH6) to LOW
  gpio_bit_reset(CTRLGBA,GBA_RD);

  // Delay here or read error with repro
  delay_GBA();  
  delay_GBA();  
  delay_GBA();

  word myWord = GPIO_ISTAT(ADDR_1)&0xFFFF;
  //printf("-%04x\n",myWord);
  myWord = ((myWord << 8) + (myWord >> 8))&0xF0FF;
  myWord += (GPIO_ISTAT(ADDR_2)&0x0F00);

  // Switch RD(PH6) to HIGH
  gpio_bit_set(CTRLGBA,GBA_RD|CS_ROM);

  //delay_GBA();  
  //gpio_bit_set(CTRLGBA,CS_ROM);
  //delay_GBA();  
  return myWord;
}



word readWord_buf_GBA(unsigned long Address, uint16_t *outBuf, uint16_t cnt) 
{


  // Divide address by two to get word addressing
  unsigned long myAddress = Address >> 1;


  // Set address/data ports to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;


  // Output address to address pins,
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);
  GPIO_OCTL(ADDR_3) = (GPIO_OCTL(ADDR_3)&0xFFFF00FF) + ((myAddress >> 8)&0xFF00);

  // Pull CS(PH3) to LOW
  gpio_bit_reset(CTRLGBA,CS_ROM);

  delay_GBA();
  delay_GBA();

  // Set address/data ports to input
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF);
  GPIO_CTL1(ADDR_1) = 0x44444444;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x44440000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x4444;//A8-A11

  //gpio_init(ADDR_1,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,BITS(4,15));
  //gpio_init(ADDR_2,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,BITS(8,11));
  //gpio_init(ADDR_3,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,BITS(8,15));

    
  //delay_GBA(); 

  for(WORD i = 0;i<cnt;i++)
  {
    // Pull RD(PH6) to LOW
    gpio_bit_reset(CTRLGBA,GBA_RD);

    // Delay here or read error with repro
    delay_GBA();  

    word myWord = GPIO_ISTAT(ADDR_1)&0xFFFF;
    //printf("-%04x\n",myWord);
    myWord = ((myWord << 8) + (myWord >> 8))&0xF0FF;
    myWord += (GPIO_ISTAT(ADDR_2)&0x0F00);

    // Switch RD(PH6) to HIGH
    gpio_bit_set(CTRLGBA,GBA_RD);

    outBuf[i] = myWord;
    //delay_GBA();  
    //delay_GBA();  

  }
  gpio_bit_set(CTRLGBA,CS_ROM);
  //delay_GBA();  
  return cnt;
}

void writeWord_GBA(unsigned long Address, word myWord) 
{

  // Divide address by two to get word addressing
  unsigned long myAddress = Address >> 1;


  // Set address/data ports to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;
  //gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,BITS(4,15));
  //gpio_init(ADDR_2,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,BITS(8,11));
  //gpio_init(ADDR_3,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,BITS(8,15));

  // Output address to address pins,
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);
  GPIO_OCTL(ADDR_3) = (GPIO_OCTL(ADDR_3)&0xFFFF00FF) + ((myAddress >> 8)&0xFF00);

  // Pull CS(PH3) to LOW
  gpio_bit_reset(CTRLGBA,CS_ROM);

  //__asm__("nop\n\t""nop\n\t");
  delay_GBA();
  delay_GBA();
  delay_GBA();

  
  #ifdef TEST_MY_CART
  //delayMicroseconds(1);
  #endif

  // Output data
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xF) + (((myWord << 8) + (myWord >> 8)) & 0xFFF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xF0FF) + (myWord&0x0F00);

  // Pull WR(PH5) to LOW
  gpio_bit_reset(CTRLGBA,GBA_WR);

  //__asm__("nop\n\t""nop\n\t");
  delay_GBA();
  delay_GBA();  
  delay_GBA();  
  delay_GBA();
  // Switch WR(PH5) to HIGH
  gpio_bit_set(CTRLGBA,GBA_WR);
  // Switch CS_ROM(PH3) to HIGH
  delay_GBA();
  gpio_bit_set(CTRLGBA,CS_ROM);
  //delay_GBA();    
 
}

// This function swaps bit at positions p1 and p2 in an integer n
word swapBits(word n, word p1, word p2)
{
  // Move p1'th to rightmost side
  word bit1 =  (n >> p1) & 1;

  // Move p2'th to rightmost side
  word bit2 =  (n >> p2) & 1;

  // XOR the two bits */
  word x = (bit1 ^ bit2);

  // Put the xor bit back to their original positions
  x = (x << p1) | (x << p2);

  // XOR 'x' with the original number so that the two sets are swapped
  word result = n ^ x;

  return result;
}



// Some repros have D0 and D1 switched
word readWord_GAB(unsigned long myAddress) {
#ifdef TEST_MY_CART
  word tempWord = readWord_GBA(myAddress);
#else
  word tempWord = swapBits(readWord_GBA(myAddress), 0, 1);
#endif
  return tempWord;
}

void writeWord_GAB(unsigned long myAddress, word myWord) {
#ifdef TEST_MY_CART
  writeWord_GBA(myAddress, myWord);
#else
  writeWord_GBA(myAddress, swapBits(myWord, 0, 1));
#endif
}


void setAddrOutMode()
{
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
}


byte readByte_GBA(unsigned long myAddress) 
{
  // Set address ports to output
  // Set data port to input
  GPIO_CTL1(ADDR_3) = 0x44444444;

  // Output address to address pins,
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);

  // Pull OE_SRAM(PH6) to LOW
  // Pull CE_SRAM(PH0) to LOW
  gpio_bit_reset(CTRLGBA,CS_SRAM|GBA_RD);

  // Hold address for at least 25ns and wait 150ns before access
  delay_GBA();
  delay_GBA();
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read byte
  byte tempByte = GPIO_ISTAT(ADDR_3)>>8;

  // Pull CE_SRAM(PH0) HIGH
  // Pull OE_SRAM(PH6) HIGH
  gpio_bit_set(CTRLGBA,GBA_RD|CS_SRAM);
  return tempByte;
}

void writeByte_GBA(unsigned long myAddress, byte myData) 
{
  // Set address ports to output
  // Set data port to output
  GPIO_CTL1(ADDR_3) = 0x33333333;

  // Output address to address pins
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);

  // Output data to data pins
  GPIO_OCTL(ADDR_3) = (GPIO_OCTL(ADDR_3)&0xFFFF00FF) + (myData << 8);

  // Wait till output is stable
  delay_GBA();

  // Pull WE_SRAM(PH5) to LOW
  // Pull CE_SRAM(PH0) to LOW
  gpio_bit_reset(CTRLGBA,GBA_WR|CS_SRAM);

  // Leave WR low for at least 60ns
  delay_GBA();
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull CE_SRAM(PH0) HIGH
  // Pull WE_SRAM(PH5) HIGH
  gpio_bit_set(CTRLGBA,GBA_WR|CS_SRAM);

  // Leave WR high for at least 50ns
  delay_GBA();
  //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}


/******************************************
  GBA ROM Functions
*****************************************/
// Read info out of rom header
void getCartInfo_GBA() 
{
  // Read Header into array
  OledClear();

  for (int currWord = 0; currWord < 96; currWord++) 
  {
    word tempWord = readWord_GBA(currWord<<1);
    //printf("%04x\n",tempWord);
    ((word *)sdBuffer)[currWord] = tempWord;
  }

  // Compare Nintendo logo against known checksum, 156 bytes starting at 0x04
  word logoChecksum = 0;
  for (int currByte = 0x4; currByte < 0xA0; currByte++) 
  {
    logoChecksum += sdBuffer[currByte];
  }

  printf("\r\ncksum = %04x\r\n",logoChecksum);

  if (logoChecksum != 0x4B1B) 
  {
    print_Error("CARTRIDGE ERROR", false);
    strcpy(romName, "ERROR");
    OledShowString(0,1,"Press OK Button to\nignore or powercycle\nto try again!",8);
    WaitOKBtn();
  }
  else 
  {
    byte tb;
    char tempStr[5] = {0};
    FIL tf;
    UINT rdt;

    // cart not in list
    cartSize = 0;
    saveType = 0;

    // Get cart ID
    cartID[0] = (char)sdBuffer[0xAC];
    cartID[1] = (char)sdBuffer[0xAD];
    cartID[2] = (char)sdBuffer[0xAE];
    cartID[3] = (char)sdBuffer[0xAF];
    cartID[4] = 0x00;

    f_chdir("/");
    if (f_open(&tf,"gba.txt", FA_READ) == FR_OK) 
    {
      // Loop through file

      while (f_eof(&tf) == false) 
      {
        // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up

        if(f_read(&tf,tempStr,4,&rdt)!=FR_OK)
        {
          //
          f_close(&tf);
          break;
        }

        // Check if string is a match
        if (strcmp(tempStr, cartID) == 0) 
        {
          // Skip the , in the file
          f_lseek(&tf,f_tell(&tf) + 1);

          // Read the next ascii character and subtract 48 to convert to decimal
          f_read(&tf,&tb,1,&rdt);
          cartSize = tb - 48;
          f_read(&tf,&tb,1,&rdt);
          // Remove leading 0 for single digit cart sizes
          if (cartSize != 0) 
          {
            cartSize = cartSize * 10 + tb - 48;
          }
          else {
            cartSize = tb - 48;
          }

          // Skip the , in the file
          f_lseek(&tf,f_tell(&tf) + 1);
          f_read(&tf,&tb,1,&rdt);

          // Read the next ascii character and subtract 48 to convert to decimal
          saveType = tb - 48;
        }
        // If no match, empty string, advance by 7 and try again
        else 
        {
          f_lseek(&tf,f_tell(&tf) + 7);
        }
      }
      // Close the file:
      f_close(&tf);
    }
    else 
    {
      print_Error("GBA.txt missing", false);
    }

    // Get name
    byte myByte = 0;
    byte myLength = 0;
    for (int addr = 0xA0; addr <= 0xAB; addr++) 
    {
      myByte = sdBuffer[addr];
      if (((myByte >= 48 && myByte <= 57) || (myByte >= 65 && myByte <= 122)) && myLength < 15) {
        romName[myLength] = myByte;
        myLength++;
      }
    }

    // Get ROM version
    romVersion = sdBuffer[0xBC];

    // Get Checksum as string
    sprintf(checksumStr, "%02X", sdBuffer[0xBD]);

    // Calculate Checksum
    int calcChecksum = 0x00;
    for (int n = 0xA0; n < 0xBD; n++) {
      calcChecksum -= sdBuffer[n];
    }
    calcChecksum = (calcChecksum - 0x19) & 0xFF;
    // Turn into string
    sprintf(calcChecksumStr, "%02X", calcChecksum);

    // Compare checksum
    if (strcmp(calcChecksumStr, checksumStr) != 0) 
    {
      OledShowString(0,0,"Result: ",8);
      OledShowString(0,60,calcChecksumStr,8);
      print_Error("Checksum Error!", false);
      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();
    }
  }
}



// Dump ROM
void readROM_GBA() 
{
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // create a new folder for the rom file
  foldern = load_dword();
  sprintf(folder, "/GBA/ROM/%s/%d", romName, foldern);
  my_mkdir(folder);
  f_chdir(folder);

  //clear the screen
  OledClear();
  OledShowString(0,0,"Saving to :",8);
  OledShowString(0,1,folder,8);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  save_dword(foldern);

  FIL tf;
  //open file on sd card
  if (f_open(&tf,fileName, FA_CREATE_ALWAYS|FA_WRITE) != FR_OK) 
  {
    print_Error("Can't create file!", true);
  }

  // Read rom
  for (int myAddress = 0; myAddress < cartSize; myAddress += 512) {
    // Blink led
    if (myAddress % 16384 == 0)
    {
      LED_RED_BLINK;
      showPersent(myAddress,cartSize,20,3);
    }

    for (int currWord = 0; currWord < 256; currWord++) 
    {
      ((word *)sdBuffer)[currWord] = readWord_GBA(myAddress + currWord*2);
    }

    // Write to SD
    UINT wdt;
    f_write(&tf, sdBuffer, 512, &wdt);
  }

  showPersent(1,1,20,3);

  // Close the file:
  f_close(&tf);
}

// Calculate the checksum of the dumped rom
boolean compare_checksum_GBA () 
{
  OledShowString(0,4,"Calculating Checksum",8);

  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // last used rom folder
  foldern = load_dword();
  sprintf(folder, "/GBA/ROM/%s/%d", romName, foldern - 1);
  f_chdir(folder);


  FIL tf;
  // If file exists
  if (f_open(&tf, fileName, FA_READ) == FR_OK) 
  {
    // Read rom header
    UINT rdt;
    f_read(&tf,sdBuffer, 512,&rdt);
    f_close(&tf);

    // Calculate Checksum
    int calcChecksum = 0x00;
    for (int n = 0xA0; n < 0xBD; n++) {
      calcChecksum -= sdBuffer[n];
    }
    calcChecksum = (calcChecksum - 0x19) & 0xFF;

    // Turn into string
    sprintf(calcChecksumStr, "%02X", calcChecksum);

    if (strcmp(calcChecksumStr, checksumStr) == 0) 
    {
      OledShowString(0,5,"Checksum matches",8);
      return 1;
    }
    else 
    {
      OledShowString(0,5,"Result: ",8);
      OledShowString(50,5,calcChecksumStr,8);
      print_Error("Checksum Error", false);
      return 0;
    }
  }
  // Else show error
  else {
    print_Error("Failed to open rom.", false);
    return 0;
  }
}





/******************************************
  GBA SRAM SAVE Functions
*****************************************/
void readSRAM_GBA(boolean browseFile, unsigned long sramSize, uint32_t pos) 
{
  if (browseFile) 
  {
    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".srm");

    // create a new folder for the save file
    foldern = load_dword();
    sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
    my_mkdir(folder);
    f_chdir(folder);

    // Save location
    OledShowString(0,0,"Saving to :",8);
    OledShowString(0,1,folder,8);
    // write new folder number back to eeprom
    foldern = foldern + 1;
    save_dword(foldern);
  }

  //open file on sd card
  FIL tf;
  if (f_open(&tf, fileName, FA_CREATE_ALWAYS|FA_WRITE) != FR_OK) 
  {
    print_Error("SD File Error", true);
  }

  // Seek to a new position in the file
  if (pos != 0)
    f_lseek(&tf,pos);

  setAddrOutMode();

  for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByte_GBA(currAddress + c);
    }

    // Write sdBuffer to file
    UINT wdt;
    f_write(&tf, sdBuffer, 512, &wdt);
  }
  // Close the file:
  f_close(&tf);

  // Signal end of process
  OledShowString(0,4,"Done.",8);
}

void writeSRAM_GBA(boolean browseFile, unsigned long sramSize, uint32_t pos) 
{
  if (browseFile) 
  {
    filePath[0] = '\0';
    fileBrowser("/","Select srm file:");
    // Create filepath
    OledClear();
  }

  //open file on sd card
  FIL tf;
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {

    OledShowString(0,1,"SRAM writing...",8);

    // Seek to a new position in the file
    if (pos != 0)
      f_lseek(&tf,pos);

    setAddrOutMode();

    for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) 
    {
      //fill sdBuffer
      UINT rdt;
      f_read(&tf, sdBuffer, 512, &rdt);

      for (int c = 0; c < 512; c++) 
      {
        // Write byte
        writeByte_GBA(currAddress + c, sdBuffer[c]);
      }

      showPersent(currAddress,sramSize,6,2);
    }
    // Close the file:
    f_close(&tf);
    showPersent(1,1,6,2);
    OledShowString(0,3,"finished!",8);

  }
  else 
  {
    print_Error("File doesnt exist!", false);
  }
}

unsigned long verifySRAM_GBA(unsigned long sramSize, uint32_t pos) 
{
  //open file on sd card
  FIL tf;
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {
    // Variable for errors
    writeErrors = 0;

    // Seek to a new position in the file
    if (pos != 0)
      f_lseek(&tf, pos);

    
    setAddrOutMode();

    for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
      //fill sdBuffer
      UINT rdt;
      f_read(&tf, sdBuffer, 512, &rdt);

      for (int c = 0; c < 512; c++) 
      {
        // Read byte
        byte bt = readByte_GBA(currAddress + c);
        //printf("\r\n%02x",bt);
        if (bt != sdBuffer[c]) {
          writeErrors++;
        }
      }
    }
    // Close the file:
    f_close(&tf);
    return writeErrors;
  }
  else 
  {
    print_Error("Can't open file!", false);
  }
  return 0;
}




/******************************************
  GBA Eeprom SAVE Functions
*****************************************/


// Send address as bits to eeprom
void send_GBA(word currAddr, word numBits) 
{
  for (word addrBit = numBits; addrBit > 0; addrBit--) {
    // If you want the k-th bit of n, then do
    // (n & ( 1 << k )) >> k
    if (((currAddr & ( 1 << (addrBit - 1))) >> (addrBit - 1))) {
      // Set A0(PF0) to High
      gpio_bit_set(ADDR_1,GPIO_PIN_8);
    }
    else {
      // Set A0(PF0) to Low
      gpio_bit_reset(ADDR_1,GPIO_PIN_8);
    }
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);
  }
}


// Write 512K eeprom block
void writeBlock_EEP(word startAddr, word eepSize) 
{
  // Setup
  // Set A0(PF0) to Output
  gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_8|GPIO_PIN_7);
  // Set A23/D7(PC7) to Output
  //gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_7);

  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);
  // Set A0(PF0) to High
  // Set A23/D7(PC7) to High
  gpio_bit_set(ADDR_1,GPIO_PIN_8|GPIO_PIN_7);
  

  __asm__("nop\n\t""nop\n\t");

  // Write 64*8=512 bytes
  for (word currAddr = startAddr; currAddr < startAddr + 64; currAddr++) 
  {
    // Set CS_ROM(PH3) to LOW
    gpio_bit_reset(CTRLGBA,CS_ROM);

    // Send write request "10"
    // Set A0(PF0) to High
    gpio_bit_set(ADDR_1,GPIO_PIN_8);
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);
    // Set A0(PF0) to LOW
    gpio_bit_reset(ADDR_1,GPIO_PIN_8);
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);

    // Send either 6 or 14 bit address
    if (eepSize == 4) {
      send_GBA(currAddr, 6);
    }
    else {
      send_GBA(currAddr, 14);
    }

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Send data
    for (byte currByte = 0; currByte < 8; currByte++) {
      send_GBA(sdBuffer[(currAddr - startAddr) * 8 + currByte], 8);
    }

    // Send stop bit
    // Set A0(PF0) to LOW
    gpio_bit_reset(ADDR_1,GPIO_PIN_8);
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);

    // Set CS_ROM(PH3) to High
    gpio_bit_set(CTRLGBA,CS_ROM);

    // Wait until done
    // Set A0(PF0) to Input
    gpio_init(ADDR_1,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_8);

    do {
      // Set  CS_ROM(PH3) RD(PH6) to LOW
      gpio_bit_reset(CTRLGBA,CS_ROM);
      // Set  CS_ROM(PH3) RD(PH6) to High
      gpio_bit_set(CTRLGBA,CS_ROM);
    }
    while (((GPIO_ISTAT(ADDR_1) >> 8) & 0x1) == 0);

    // Set A0(PF0) to Output
    gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_8);
  }
}

// Reads 512 bytes from eeprom
void readBlock_EEP(word startAddress, word eepSize) {
  // Setup
  // Set A0(PF0) to Output
  // Set A23/D7(PC7) to Output
  gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_8|GPIO_PIN_7);

  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);
  // Set A0(PF0) to High
  // Set A23/D7(PC7) to High
  gpio_bit_set(ADDR_1,GPIO_PIN_8|GPIO_PIN_7);

  __asm__("nop\n\t""nop\n\t");

  // Read 64*8=512 bytes
  for (word currAddr = startAddress; currAddr < startAddress + 64; currAddr++) 
  {
    // Set CS_ROM(PH3) to LOW
    gpio_bit_reset(CTRLGBA,CS_ROM);

    // Send read request "11"
    // Set A0(PF0) to High
    gpio_bit_set(ADDR_1,GPIO_PIN_8);
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);

    // Send either 6 or 14 bit address
    if (eepSize == 4) {
      send_GBA(currAddr, 6);
    }
    else {
      send_GBA(currAddr, 14);
    }

    // Send stop bit
    // Set A0(PF0) to LOW
    gpio_bit_reset(ADDR_1,GPIO_PIN_8);
    // Set WR(PH5) to LOW
    gpio_bit_reset(CTRLGBA,GBA_WR);
    // Set WR(PH5) to High
    gpio_bit_set(CTRLGBA,GBA_WR);

    // Set CS_ROM(PH3) to High
    gpio_bit_set(CTRLGBA,CS_ROM);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read data
    // Set A0(PF0) to Input
    gpio_init(ADDR_1,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_8);
    // Set CS_ROM(PH3) to low
    gpio_bit_reset(CTRLGBA,CS_ROM);

    // Array that holds the bits
    bool tempBits[65];

    // Ignore the first 4 bits
    for (byte i = 0; i < 4; i++) 
    {
      // Set RD(PH6) to LOW
      gpio_bit_reset(CTRLGBA,GBA_RD);
      // Set RD(PH6) to High
      gpio_bit_set(CTRLGBA,GBA_RD);
    }

    // Read the remaining 64bits into array
    for (byte currBit = 0; currBit < 64; currBit++) 
    {
      // Set RD(PH6) to LOW
      gpio_bit_reset(CTRLGBA,GBA_RD);
      // Set RD(PH6) to High
      gpio_bit_set(CTRLGBA,GBA_RD);

      // Read bit from A0(PF0)
      tempBits[currBit] = ((GPIO_ISTAT(ADDR_1) >> 8) & 0x1);
    }

    // Set CS_ROM(PH3) to High
    gpio_bit_set(CTRLGBA,CS_ROM);
    // Set A0(PF0) to Output
    gpio_init(ADDR_1,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_8);
    // Set A0(PF0) to High
    gpio_bit_set(ADDR_1,GPIO_PIN_8);


    // OR 8 bits into one byte for a total of 8 bytes
    for (byte j = 0; j < 64; j += 8) {
      sdBuffer[((currAddr - startAddress) * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
    }
  }
}




// Check if the SRAM was written without any error
unsigned long verifyEEP_GBA(word eepSize) 
{
  unsigned long wrError = 0;

  //open file on sd card
  FIL tf;
  if (f_open(&tf, filePath, FA_READ) != FR_OK) 
  {
    print_Error("SD File Error!", true);
  }

  // Fill sd Buffer
  for (word currAddress = 0; currAddress < eepSize * 16; currAddress += 64) 
  {
    // Disable interrupts for more uniform clock pulses
    __disable_irq();
    readBlock_EEP(currAddress, eepSize);
    __enable_irq();

    // Compare
    for (int currByte = 0; currByte < 512; currByte++) 
    {
      byte tb;
      UINT rdt;
      f_read(&tf,&tb,1,&rdt);
      if (sdBuffer[currByte] != tb) 
      {
        wrError++;
      }
    }
  }
  f_close(&tf);
  return wrError;
}


// Write eeprom from file
void writeEeprom_GBA(word eepSize) {
  // Launch Filebrowser
  filePath[0] = '\0';
  fileBrowser("/","Select eep file");

  OledShowString(0,0,"Writing eeprom...",8);
  
  FIL tf;
  //open file on sd card
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {
    for (word i = 0; i < eepSize * 16; i += 64) 
    {
      // Fill romBuffer
      UINT rdt;
      f_read(&tf, sdBuffer, 512, &rdt);
      // Disable interrupts for more uniform clock pulses
      __disable_irq();
      // Write 512 bytes
      writeBlock_EEP(i, eepSize);
      __enable_irq();

      // Wait
      delayMicroseconds(200);//???
    }

    // Close the file:
    f_close(&tf);
    OledShowString(0,1,"Done.",8);
    
  }
  else 
  {
    OledShowString(20,0,"Error!",8);
    print_Error("File doesnt exist!", false);
  }
}

// Read eeprom to file
void readEeprom_GBA(word eepSize) {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".eep");

  // create a new folder for the save file
  foldern = load_dword();

  sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
  my_mkdir(folder);
  f_chdir(folder);

  // Save location
  OledShowString(0,0,"Saving to :",8);
  OledShowString(0,1,folder,8);
  

  // write new folder number back to eeprom
  foldern = foldern + 1;
  save_dword(foldern);

  FIL tf;

  //open file on sd card
  if (f_open(&tf, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) 
  {
    print_Error("SD File Error!", true);
  }

  // Each block contains 8 Bytes, so for a 8KB eeprom 1024 blocks need to be read
  for (word currAddress = 0; currAddress < eepSize * 16; currAddress += 64) 
  {
    // Disable interrupts for more uniform clock pulses
    __disable_irq();
    // Fill sd Buffer
    readBlock_EEP(currAddress, eepSize);
    __enable_irq();

    UINT wdt;
    // Write sdBuffer to file
    f_write(&tf, sdBuffer, 512, &wdt);
    // Wait
    delayMicroseconds(200);
  }
  f_close(&tf);
}



/******************************************
  GBA FLASH SAVE Functions
*****************************************/
// SST 39VF512 Flashrom
byte readByteFlash_GBA(unsigned long myAddress) 
{
  // Set address
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);

  // Wait until byte is ready to read
  delay_GBA();
  delay_GBA();

  // Read byte
  byte tempByte = (GPIO_ISTAT(ADDR_3) >> 8)&0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  delay_GBA();
  delay_GBA();

  return tempByte;
}

void writeByteFlash_GBA(unsigned long myAddress, byte myData) 
{
  //
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF) + (myAddress & 0x0F00);

  GPIO_OCTL(ADDR_3) = (GPIO_OCTL(ADDR_3)&0xFFFF00FF) + (myData<<8);

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  delay_GBA();
  delay_GBA();

  // Switch WE_FLASH(PH5) to LOW
  gpio_bit_reset(CTRLGBA,GBA_WR);

  // Leave WE low for at least 40ns
  delay_GBA();
  delay_GBA();

  // Switch WE_FLASH(PH5) to HIGH
  gpio_bit_set(CTRLGBA,GBA_WR);

  // Leave WE high for a bit
  delay_GBA();
  delay_GBA();
}

// Erase FLASH
void eraseFLASH_GBA() 
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);

  // Set address ports to output
  // Set data pins to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;


  // Output a LOW signal on CE_FLASH(PH0)
  gpio_bit_reset(CTRLGBA,CS_SRAM);

  // Erase command sequence
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x80);
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x10);

  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);

  // Wait until all is erased
  delay(500);
}



void idFlash_GBA() 
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);

  // Set address ports to output
  // Set data pins to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;

  // Output a LOW signal on CE_FLASH(PH0)
  gpio_bit_reset(CTRLGBA,CS_SRAM);

  // ID command sequence
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x90);

  // Set data pins to input
  GPIO_CTL1(ADDR_3) = 0x44444444;

  // Output a LOW signal on OE_FLASH(PH6)
  gpio_bit_reset(CTRLGBA,GBA_RD);

  // Wait 150ns before reading ID
  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  // Read the two id bytes into a string
  byte bid0 = readByteFlash_GBA(0);
  byte bid1 = readByteFlash_GBA(1); 
  sprintf(flashid, "%02X%02X", bid0,bid1);

  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);
}

// Reset FLASH
void resetFLASH_GBA() 
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);

  // Set address ports to output
  // Set data pins to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;

  // Output a LOW signal on CE_FLASH(PH0)
  gpio_bit_reset(CTRLGBA,CS_SRAM);

  // Reset command sequence
  writeByteFlash_GBA(0x5555, 0xAA);
  writeByteFlash_GBA(0x2AAA, 0x55);
  writeByteFlash_GBA(0x5555, 0xf0);
  writeByteFlash_GBA(0x5555, 0xf0);

  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);

  // Wait
  delay(100);
}

boolean blankcheckFLASH_GBA (unsigned long flashSize) 
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  gpio_bit_set(CTRLGBA,GBA_WR|CS_ROM);

  // Set address ports to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  // Set address to 0
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF);

  // Set data pins to input
  GPIO_CTL1(ADDR_3) = 0x44444444;
  // Disable Pullups
  //PORTC = 0x00;

  boolean blank = 1;

  // Output a LOW signal on  CE_FLASH(PH0)
  gpio_bit_reset(CTRLGBA,CS_SRAM);

  // Output a LOW signal on OE_FLASH(PH6)
  gpio_bit_reset(CTRLGBA,GBA_RD);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    // Fill buffer
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByteFlash_GBA(currAddress + c);
    }
    // Check buffer
    for (unsigned long currByte = 0; currByte < 512; currByte++) {
      if (sdBuffer[currByte] != 0xFF) {
        currByte = 512;
        currAddress = flashSize;
        blank = 0;
      }
    }

    LED_GREEN_BLINK;
  }
  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);

  return blank;
}

// The MX29L010 is 131072 bytes in size and has 16 sectors per bank
// each sector is 4096 bytes, there are 32 sectors total
// therefore the bank size is 65536 bytes, so we have two banks in total
void switchBank_GBA(byte bankNum) 
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);

  // Set address ports to output
  // Set data pins to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;

  // Output a LOW signal on CE_FLASH(PH0)
  gpio_bit_reset(CTRLGBA,CS_SRAM);

  // Switch bank command sequence
  writeByte_GBA(0x5555, 0xAA);
  writeByte_GBA(0x2AAA, 0x55);
  writeByte_GBA(0x5555, 0xB0);
  writeByte_GBA(0x0000, bankNum);

  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);
}

void readFLASH_GBA (boolean browseFile, unsigned long flashSize, uint32_t pos)
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  gpio_bit_set(CTRLGBA,GBA_WR|CS_ROM);

  // Set address ports to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  // Set address to 0
  GPIO_OCTL(ADDR_1) = (GPIO_OCTL(ADDR_1)&0xFFFF000F);
  GPIO_OCTL(ADDR_2) = (GPIO_OCTL(ADDR_2)&0xFFFFF0FF);

  // Set data pins to input
  GPIO_CTL1(ADDR_3) = 0x44444444;

  if (browseFile) 
  {
    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".fla");

    // create a new folder for the save file
    foldern = load_dword();

    sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
    my_mkdir(folder);
    f_chdir(folder);

    // Save location
    OledShowString(0,0,"Saving to :",8);
    OledShowString(0,1,folder,8);

    // write new folder number back to eeprom
    foldern = foldern + 1;
    save_dword(foldern);
  }


  FIL tf;
  //open file on sd card
  if (f_open(&tf, fileName,FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) 
  {
    print_Error("SD File Error", true);
  }

  // Seek to a new position in the file
  if (pos != 0)
    f_lseek(&tf, pos);

  // Output a LOW signal on CE_FLASH(PH0)
  gpio_bit_reset(CTRLGBA,CS_SRAM);

  // Output a LOW signal on OE_FLASH(PH6)
  gpio_bit_reset(CTRLGBA,GBA_RD);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) 
  {
    LED_RED_BLINK;
    showPersent(currAddress,flashSize,20,3);
    for (int c = 0; c < 512; c++) 
    {
      // Read byte
      sdBuffer[c] = readByteFlash_GBA(currAddress + c);
    }
    // Write sdBuffer to file
    UINT wdt;
    f_write(&tf, sdBuffer, 512, &wdt);


  }
  showPersent(1,1,20,3);
  f_close(&tf);

  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);

  // Signal end of process
  OledShowString(20,4,"Done!",8);
}


void busyCheck_GBA(int currByte) 
{
  // Set data pins to input
  GPIO_CTL1(ADDR_3) = 0x44444444;
  // Output a LOW signal on OE_FLASH(PH6)
  gpio_bit_reset(CTRLGBA,GBA_RD);
  // Read PINC
  while (((GPIO_ISTAT(ADDR_3)>>8)&0xFF) != sdBuffer[currByte]) 
  {
  }
  // Output a HIGH signal on OE_FLASH(PH6)
  gpio_bit_set(CTRLGBA,GBA_RD);
  // Set data pins to output
  GPIO_CTL1(ADDR_3) = 0x33333333;
}

void writeFLASH_GBA (boolean browseFile, unsigned long flashSize, uint32_t pos)
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  gpio_bit_set(CTRLGBA,GBA_RD|GBA_WR|CS_ROM);

  // Set address ports to output
  // Set data port to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11
  GPIO_CTL1(ADDR_3) = 0x33333333;

  if (browseFile) 
  {
    filePath[0] = '\0';
    fileBrowser("/","Select fla file");
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    OledClear();
  }

  OledShowString(0,0,"Writing flash...",8);
   

  FIL tf;
  //open file on sd card
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {

    // Seek to a new position in the file
    if (pos != 0)
      f_lseek(&tf,pos);

    // Output a LOW signal on CE_FLASH(PH0)
    gpio_bit_reset(CTRLGBA,CS_SRAM);

    for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) 
    {
      //fill sdBuffer
      UINT rdt;
      f_read(&tf, sdBuffer, 512, &rdt);

      for (int c = 0; c < 512; c++) {
        // Write command sequence
        writeByteFlash_GBA(0x5555, 0xaa);
        writeByteFlash_GBA(0x2aaa, 0x55);
        writeByteFlash_GBA(0x5555, 0xa0);
        // Write current byte
        writeByteFlash_GBA(currAddress + c, sdBuffer[c]);

        // Wait
        busyCheck_GBA(c);
      }
    }
    // Set CS_FLASH(PH0) high
    gpio_bit_set(CTRLGBA,CS_SRAM);

    // Close the file:
    f_close(&tf);
    OledShowString(0,3,"Done!",8);     

  }
  else 
  {
    OledShowString(0,4,"Error!",8);
    print_Error("File doesnt exist!", false);
  }
}

// Check if the Flashrom was written without any error
void verifyFLASH_GBA(unsigned long flashSize, uint32_t pos) 
{
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  gpio_bit_set(CTRLGBA,GBA_WR|CS_ROM);

  // Set address ports to output
  GPIO_CTL1(ADDR_1) = 0x33333333;//A0-A7
  GPIO_CTL0(ADDR_1) = (GPIO_CTL0(ADDR_1)&0xFFFF) + 0x33330000;//A12-A15
  GPIO_CTL1(ADDR_2) = (GPIO_CTL1(ADDR_2)&0xFFFF0000) + 0x3333;//A8-A11

  // Set data pins to input
  GPIO_CTL1(ADDR_3) = 0x44444444;

  // Output a LOW signal on CE_FLASH(PH0) and  OE_FLASH(PH6)
  gpio_bit_reset(CTRLGBA,CS_SRAM|GBA_RD);

  // Signal beginning of process
  OledShowString(0,2,"Verify...",8);
   

  unsigned long wrError = 0;
  FIL tf;

  //open file on sd card
  if (f_open(&tf, filePath, FA_READ) != FR_OK)
  {
    print_Error("SD File Error!", true);
  }

  // Seek to a new position in the file
  if (pos != 0)
    f_lseek(&tf, pos);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512)
  {
    UINT rdt;
    f_read(&tf, sdBuffer, 512, &rdt);

    for (int c = 0; c < 512; c++) {
      // Read byte
      if (sdBuffer[c] != readByteFlash_GBA(currAddress + c)) {
        wrError++;
      }
    }
  }
  f_close(&tf);

  // Set CS_FLASH(PH0) high
  gpio_bit_set(CTRLGBA,CS_SRAM);

  if (wrError == 0) 
  {
    OledShowString(55,2,"OK",8);
  }
  else 
  {
    char tmsg[32] = {0};
    sprintf(tmsg," %d Errors.",wrError);
    print_Error(tmsg, false);
  }
}




/******************************************
  GBA REPRO Functions (32MB Intel 4000L0YBQ0 and 16MB MX29GL128E)

*****************************************/
unsigned long fileSize;

// Reset to read mode
void resetIntel_GBA(unsigned long partitionSize) 
{
  for (unsigned long currPartition = 0; currPartition < cartSize; currPartition += partitionSize)
  {
    writeWord_GBA(currPartition, 0xFFFF);
  }
}

void resetMX29GL128E_GBA() 
{
  writeWord_GAB(0, 0xF0);
  delay(1);
}

boolean sectorCheckMX29GL128E_GBA() {
  boolean sectorProtect = 0;
  writeWord_GAB(0xAAA, 0xAA);
  writeWord_GAB(0x555, 0x55);
  writeWord_GAB(0xAAA, 0x90);
  for (unsigned long currSector = 0x0; currSector < 0xFFFFFF; currSector += 0x20000) {
    if (readWord_GAB(currSector + 0x04) != 0x0)
      sectorProtect = 1;
  }
  resetMX29GL128E_GBA();
  return sectorProtect;
}

void idFlashrom_GBA() 
{
  // Send Intel ID command to flashrom
  writeWord_GBA(0, 0x90);
  delay_GBA();
  delay_GBA();
  delay_GBA();
  delay_GBA();

  // Read flashrom ID
  sprintf(flashid, "%02X%02X", ((readWord_GBA(0x2) >> 8) & 0xFF), (readWord_GBA(0x4) & 0xFF));

  // Intel Strataflash
  if (strcmp(flashid, "8802") == 0 || (strcmp(flashid, "8816") == 0)) 
  {
    cartSize = 0x2000000;
  }
  else {
    // Send swapped MX29GL128E/MSP55LV128 ID command to flashrom

    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x90);
    
    delay_GBA();
    delay_GBA();
    delay_GBA();
    delay_GBA();
    // Read flashrom ID
    sprintf(flashid, "%02X%02X", ((readWord_GAB(0x2) >> 8) & 0xFF), (readWord_GAB(0x2) & 0xFF));

    // MX29GL128E or MSP55LV128
    if (strcmp(flashid, "227E") == 0) {
      // MX is 0xC2 and MSP is 0x4 or 0x1
      romType = (readWord_GAB(0x0) & 0xFF);
      cartSize = 0x1000000;
      resetMX29GL128E_GBA();
    }
    else 
    {
      char tmsg[64] = {0};
      sprintf(tmsg,"Error!\nUnknown Flash!\nFlash ID: %s",flashid);
      OledShowString(0,0,tmsg,8);
      print_Error("Check voltage?", true);
    }
  }
}

boolean blankcheckFlashrom_GBA() 
{
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x20000) 
  {
    // Blink led
    LED_GREEN_BLINK;

    for (unsigned long currByte = 0; currByte < 0x20000; currByte += 2) {
      if (readWord_GBA(currSector + currByte) != 0xFFFF) 
      {
        return 0;
      }
    }
  }
  return 1;
}

void eraseIntel4000_GBA() 
{
  // If the game is smaller than 16Mbit only erase the needed blocks
  unsigned long lastBlock = 0xFFFFFF;
  if (fileSize < 0xFFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }

    LED_RED_BLINK;
    showPersent(currBlock,lastBlock,70,2);
  }

  // Erase 126 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) 
  {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
    // Blink led
    LED_RED_BLINK;
    showPersent(currBlock,lastBlock,70,2);
  }

  showPersent(1,1,70,2);

  // Erase the second chip
  if (fileSize > 0xFFFFFF) {
    // 126 blocks with 64kwords each
    for (unsigned long currBlock = 0x1000000; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      LED_RED_BLINK;
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      LED_RED_BLINK;
    }
  }
}

void eraseIntel4400_GBA() 
{
  // If the game is smaller than 32Mbit only erase the needed blocks
  unsigned long lastBlock = 0x1FFFFFF;
  if (fileSize < 0x1FFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }

    LED_RED_BLINK;
    showPersent(currBlock,lastBlock,70,2);
  }

  // Erase 255 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF)
  {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
    // Blink led
    LED_RED_BLINK;
    showPersent(currBlock,lastBlock,70,2);
  }

  showPersent(1,1,70,2);

  /* No need to erase the second chip as max rom size is 32MB
    if (fileSize > 0x2000000) {
    // 255 blocks with 64kwords each
    for (unsigned long currBlock = 0x2000000; currBlock < 0x3FDFFFF; currBlock += 0x1FFFF) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      LED_RED_BLINK;
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x3FE0000; currBlock < 0x3FFFFFF; currBlock += 0x8000) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      LED_RED_BLINK;
    }
    }*/
}

#define deley_us_lv128 (1)

void sectorEraseMSP55LV128_GBA() 
{
  unsigned long lastSector = 0xFFFFFF;

  // Erase 256 sectors with 64kbytes each
  unsigned long currSector;
  for (currSector = 0x0; currSector < lastSector; currSector += 0x10000) {
    writeWord_GAB(0xAAA, 0xAA);
    delayMicroseconds(deley_us_lv128);   
    writeWord_GAB(0x555, 0x55);
    delayMicroseconds(deley_us_lv128);   
    writeWord_GAB(0xAAA, 0x80);
    delayMicroseconds(deley_us_lv128);   
    writeWord_GAB(0xAAA, 0xAA);
    delayMicroseconds(deley_us_lv128);   
    writeWord_GAB(0x555, 0x55);
    delayMicroseconds(deley_us_lv128);     
    writeWord_GAB(currSector, 0x30);
    delayMicroseconds(deley_us_lv128);    

    // Blink LED
    LED_RED_BLINK;
    showPersent(currSector,lastSector,68,2);

    // Read the status register
    word statusReg = readWord_GAB(currSector);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
    
      delayMicroseconds(deley_us_lv128);        
      statusReg = readWord_GAB(currSector);
    }

    delayMicroseconds(deley_us_lv128); 

  }

   showPersent(1,1,68,2);
}




void sectorEraseMX29GL128E_GBA() {
  unsigned long lastSector = 0xFFFFFF;

  // Erase 128 sectors with 128kbytes each
  unsigned long currSector;
  for (currSector = 0x0; currSector < lastSector; currSector += 0x20000) {
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x80);
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(currSector, 0x30);
    // Blink LED
    LED_RED_BLINK;
    showPersent(currSector,lastSector,68,2);
    // Read the status register
    word statusReg = readWord_GAB(currSector);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GAB(currSector);
    }

  }

  showPersent(1,1,68,2);
}

void writeIntel4000_GBA(FIL * ptf) 
{
  for (unsigned long currBlock = 0; currBlock < fileSize; currBlock += 0x20000) 
  {
    // Blink led
    LED_BLUE_BLINK;
    showPersent(currBlock,fileSize,68,3);
    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512) 
    {
      // Fill SD buffer
      UINT rdt;
      f_read(ptf, sdBuffer, 512, &rdt);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Unlock Block
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0x60);
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0xD0);

        // Buffered program command
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0xE8);

        // Check Status register
        word statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer);
        }

        // Write word count (minus 1)
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0x1F);

        // Write buffer
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          word currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Write Buffer to Flash
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62, 0xD0);

        // Read the status register at last written address
        statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          delay_GBA();
          statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62);
        }
      }
    }
  }
  showPersent(1,1,68,3);
}






void writeMSP55LV128_GBA(FIL * ptf) 
{

  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x10000) 
  {
    // Blink led
    LED_BLUE_BLINK;
    showPersent(currSector,fileSize,68,3);
    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x10000; currSdBuffer += 512) 
    {
      // Fill SD buffer
      UINT rdt;
      if(f_read(ptf, sdBuffer, 512, &rdt)!= FR_OK){printf("\nF_read err!");};

      // Write 16 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32) 
      {

        _reProgram:
        // Write Buffer command
        writeWord_GAB(0xAAA, 0xAA);
        delayMicroseconds(deley_us_lv128);
        writeWord_GAB(0x555, 0x55);
        delayMicroseconds(deley_us_lv128);
        writeWord_GAB(currSector, 0x25);
        delayMicroseconds(deley_us_lv128);

        // Write word count (minus 1)
        writeWord_GAB(currSector, 0xF);

        // Write buffer
        word currWord;
        for (byte currByte = 0; currByte < 16; currByte++) 
        {
          // Join two bytes into one word
          //delay_GBA();
          delayMicroseconds(deley_us_lv128);
          currWord = ((word *)sdBuffer)[(currWriteBuffer>>1) + currByte];
          writeWord_GBA(currSector + currSdBuffer + currWriteBuffer + currByte*2, currWord);
        }

        delayMicroseconds(deley_us_lv128);
        delayMicroseconds(deley_us_lv128);
        // Confirm write buffer
        writeWord_GAB(currSector, 0x29);

        delayMicroseconds(deley_us_lv128);
        delayMicroseconds(deley_us_lv128);


        // Read the status register
        word statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
       // int i= 0;

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) 
        {
          //delay(1);//Microseconds(600);)
          delayMicroseconds(deley_us_lv128);          
          //statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);


          //
          //i++;
          //if(i < 100)
          //{
          //  //
          //  statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
          //  continue;
          //}
                   
          if(statusReg&0x22)
          {
            
            statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
            delayMicroseconds(deley_us_lv128);
            
            if((statusReg | 0xFF7F) != (currWord | 0xFF7F))
            {
              //
              if(statusReg&0x20)
              {
                //reset
                //writeWord_GAB(0, 0xF0);
                                //write buffer abort reset
                writeWord_GAB(0xAAA, 0xAA);
                delayMicroseconds(deley_us_lv128);
                writeWord_GAB(0x555, 0x55);
                delayMicroseconds(deley_us_lv128);
                writeWord_GAB(0xAAA, 0xF0);

                delay(1000);
                printf("write err1!\n");

                LED_BLUE_BLINK;
                goto _reProgram;
              }
              else
              if(statusReg&0x2)
              {
                //write buffer abort reset
                writeWord_GAB(0xAAA, 0xAA);
                delayMicroseconds(deley_us_lv128);
                writeWord_GAB(0x555, 0x55);
                delayMicroseconds(deley_us_lv128);
                writeWord_GAB(0xAAA, 0xF0);

                delay(1000);
                printf("write err2!\n");

                LED_BLUE_BLINK;
                goto _reProgram;
              }
            }
            else break;
          }
          else
          {
            //
            statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
          }
        }

        delayMicroseconds(deley_us_lv128); 


      }

      //delay(1);
    }
  }
  showPersent(1,1,68,3);
}

void writeMX29GL128E_GBA(FIL * ptf) 
{
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x20000) 
  {
    // Blink led
    LED_BLUE_BLINK;
    showPersent(currSector,fileSize,68,3);
    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512) 
    {
      // Fill SD buffer
      UINT rdt;
      f_read(ptf, sdBuffer, 512, &rdt);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Write Buffer command
        writeWord_GAB(0xAAA, 0xAA);
        writeWord_GAB(0x555, 0x55);
        writeWord_GAB(currSector, 0x25);

        // Write word count (minus 1)
        writeWord_GAB(currSector, 0x1F);

        // Write buffer
        word currWord;
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Confirm write buffer
        writeWord_GAB(currSector, 0x29);

        // Read the status register
        word statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          delay_GBA();
          statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);
          
        }
      }
    }
  }
  showPersent(1,1,68,3);
}

boolean verifyFlashrom_GBA() 
{
  // Open file on sd card
  FIL tf;
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {
    writeErrors = 0;

    for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) 
    {
      // Blink led
      LED_GREEN_BLINK;
      showPersent(currSector,fileSize,82,6);
      for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) 
      {
        // Fill SD buffer
        UINT rdt;
        f_read(&tf, sdBuffer, 512, &rdt);

        for (int currByte = 0; currByte < (512>>1); currByte ++) 
        {
          // Join two bytes into one word
          word currWord =  ((word *)sdBuffer)[currByte];

          delayMicroseconds(1);

          // Compare both
          if (readWord_GBA(currSector + currSdBuffer + currByte * 2) != currWord) {
            writeErrors++;
            f_close(&tf);
            return 0;
          }
        }
      }
    }

    showPersent(1,1,82,6);
    // Close the file:
    f_close(&tf);
    if (writeErrors == 0) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else 
  {
    print_Error("Can't open file!", true);
    return true;
  }
}


uint16_t tbuf[256] = {0};
boolean verifyFlashrom_GBA_new() 
{
  // Open file on sd card
  FIL tf;
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {
    writeErrors = 0;

    for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) 
    {
      // Blink led
      LED_GREEN_BLINK;
      showPersent(currSector,fileSize,82,5);
      for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) 
      {
        // Fill SD buffer
        UINT rdt;
        f_read(&tf, sdBuffer, 512, &rdt);
        readWord_buf_GBA(currSector + currSdBuffer,tbuf,256);

        for (int i = 0; i < (512>>1); i++) 
        {
          // Join two bytes into one word
          word currWord =  ((word *)sdBuffer)[i];
          if (tbuf[i] != currWord) {
            writeErrors++;
            f_close(&tf);
            return 0;
          }
        }
      }
    }

    showPersent(1,1,82,5);
    // Close the file:
    f_close(&tf);
    if (writeErrors == 0) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else 
  {
    print_Error("Can't open file!", true);
    return true;
  }
}




void flashRepro_GBA() 
{
  // Check flashrom ID's
  char tmsg[100] = {0};

  uint32_t use_tick = getSystick();

  OledClear();
  idFlashrom_GBA();

  if ((strcmp(flashid, "8802") == 0) || (strcmp(flashid, "8816") == 0) || (strcmp(flashid, "227E") == 0)) 
  {
    sprintf(tmsg,"ID:%s size:%d MB.",flashid,cartSize / 0x100000);
    // MX29GL128E or MSP55LV128(N)
    if (strcmp(flashid, "227E") == 0) 
    {
      // MX is 0xC2 and MSP55LV128 is 0x4 and MSP55LV128N 0x1
      if (romType == 0xC2) {
        strcat(tmsg,"\n Macronix\n MX29GL128E");
      }
      else if ((romType == 0x1) || (romType == 0x4)) {
        strcat(tmsg,"\n Fujitsu\n MSP55LV128N");
      }
      else if (romType == 0x89) {
        strcat(tmsg,"\n Intel\n PC28F256M29");
      }
      else if (romType == 0x20) {
        strcat(tmsg,"\n ST\n M29W128GH");
      }
      else {
        sprintf(tmsg,"%s\n RomType : 0x%04x",tmsg,romType);
        OledShowString(0,0,tmsg,8);
        print_Error("Unknown manufacturer", true);
      }
    }
    // Intel 4000L0YBQ0
    else if (strcmp(flashid, "8802") == 0) {
      strcat(tmsg,"\n Intel\n 4000L0YBQ0");
    }
    // Intel 4400L0ZDQ0
    else if (strcmp(flashid, "8816") == 0) {
      strcat(tmsg,"\n Intel\n 4400L0ZDQ0");
    }

    strcat(tmsg,"\n\nThis will erase your\nRepro Cartridge.\nPress OK Button...");
    OledShowString(0,0,tmsg,8);
    WaitOKBtn();

    // Launch file browser
    filePath[0] = '\0';
    fileBrowser("/","Select gba file:");
    OledClear();

    FIL tf;
    // Open file on sd card
    if (f_open(&tf, filePath, FA_READ) == FR_OK) 
    {
      // Get rom size from file
      fileSize = f_size(&tf);
      sprintf(tmsg,"FileSize: %dMB",fileSize / 0x100000);
      OledShowString(0,0,tmsg,8);

      // Erase needed sectors
      if (strcmp(flashid, "8802") == 0) {
        OledShowString(10,2,"Erasing...",8);

        eraseIntel4000_GBA();
        resetIntel_GBA(0x200000);
      }
      else if (strcmp(flashid, "8816") == 0) {
        OledShowString(10,2,"Erasing...",8);
        eraseIntel4400_GBA();
        resetIntel_GBA(0x200000);
      }
      else if (strcmp(flashid, "227E") == 0) {
        //if (sectorCheckMX29GL128E_GBA()) {
        //print_Error(F("Sector Protected"), true);
        //}
        //else {
        OledShowString(10,2,"Erasing...",8);

        if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20)) {
          //MX29GL128E
          //PC28F256M29 (0x89)
          
          sectorEraseMX29GL128E_GBA();
          
        }
        else if ((romType == 0x1) || (romType == 0x4)) {
          //MSP55LV128(N)
          //#ifndef TEST_MY_CART
          sectorEraseMSP55LV128_GBA();
          //#endif
        }
        //}
      }
      
      // Skip blankcheck to save time
      
        //if (blankcheckFlashrom_GBA() == false) {
        //OledClear();
          //OledShowString(10,2,"Checking Err!",8);
          //f_close(&tf);
          //print_Error("cart not empty!",true);
        //}

      //Write flashrom
      OledShowString(10,3,"Writing...",8);
      OledShowString(0,4,filePath,8);

      if ((strcmp(flashid, "8802") == 0) || (strcmp(flashid, "8816") == 0)) 
      {
        OledShowString(0,1,"Intel 4x00",8);
        writeIntel4000_GBA(&tf);
      }
      else if (strcmp(flashid, "227E") == 0) 
      {
        if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20)) {
          //MX29GL128E (0xC2)
          //PC28F256M29 (0x89)
          OledShowString(0,1,"29 GL",8);
          writeMX29GL128E_GBA(&tf);
        }
        else if ((romType == 0x1) || (romType == 0x4)) {
          //MSP55LV128(N)
          OledShowString(0,1,"MSP55LV",8);
          writeMSP55LV128_GBA(&tf);
        }
      }

      // Close the file:
      f_close(&tf);

      // Verify
      OledShowString(10,5,"Verifying...",8);

      if (strcmp(flashid, "8802") == 0) 
      {
        // Don't know the correct size so just take some guesses
        resetIntel_GBA(0x8000);
        delay(1000);
        resetIntel_GBA(0x100000);
        delay(1000);
        resetIntel_GBA(0x200000);
        delay(1000);
      }
      else if (strcmp(flashid, "8816") == 0) 
      {
        resetIntel_GBA(0x200000);
        delay(1000);
      }
      else if (strcmp(flashid, "227E") == 0) 
      {
        resetMX29GL128E_GBA();
        delay(1000);
      }



      if (verifyFlashrom_GBA_new() == 1) 
      {
        //OledShowString(74,6,"OK!",8);
      }
      else 
      {
        OledClear();
        print_Error("verify ERROR!", true);
      }
      /* Skipped blankcheck
        }
        else {
        print_Error(F("failed"), true);
        }
      */


      use_tick = (getSystick() - use_tick)/1000;
      sprintf(tmsg,"Use Time: %d(s)",use_tick);
      OledShowString(10,6,tmsg,8);
    }
    else {
      print_Error("Can't open file!", true);
    }
  }
  else 
  {
    sprintf(tmsg,"Error!\nUnknown Flash!\nFlash ID: %s",flashid);
    OledShowString(0,0,tmsg,8);
    print_Error("Check voltage?", true);
  }
}



/******************************************
   Setup
 *****************************************/
void setup_GBA() 
{
  //
  char tmsg[64] = {0};

  setROM_GBA();

  // Print start page
  getCartInfo_GBA();
  OledClear();

  OledShowString(0,0,"Name: ",8);
  OledShowString(0,40,romName,8);
  OledShowString(0,1,"Cart ID: ",8);
  OledShowString(60,1,cartID,8);
  OledShowString(0,2,"Rom Size: ",8);
  if (cartSize == 0)
    OledShowString(64,2,"Unknown",8);
  else 
  {
    //
    sprintf(tmsg,"%d MB",cartSize);    
    OledShowString(64,2,tmsg,8);
  }
  strcpy(tmsg,"Save: ");
  switch (saveType)
  {
    case 0:
      strcat(tmsg,"Unknown");
      break;

    case 1:
      strcat(tmsg,"4K Eeprom");
      break;

    case 2:
      strcat(tmsg,"64K Eeprom");
      break;

    case 3:
      strcat(tmsg,"256K Sram");
      break;

    case 4:
      strcat(tmsg,"512K Flash");
      break;

    case 5:
      strcat(tmsg,"1024K Flash");
      break;
  }
  OledShowString(0,3,tmsg,8);

  sprintf(tmsg,"Checksum: %s\nVersion: 1.%d",checksumStr,romVersion);
  OledShowString(0,4,tmsg,8);

  // Wait for user input
  OledShowString(0,7,"Press OK Button...",8);
  WaitOKBtn();
}



/******************************************
   Menu
 *****************************************/
// GBA menu items
static const char GBAMenuItem1[] = "Flash GBA Cart";
static const char GBAMenuItem2[] = "Read Rom";
static const char GBAMenuItem3[] = "Read Save";
static const char GBAMenuItem4[] = "Write Save";
static const char GBAMenuItem5[] = "Force Savetype";
static const char GBAMenuItem6[] = "Reset";
static const char* const menuOptionsGBA[] = {GBAMenuItem1, GBAMenuItem2, GBAMenuItem3, GBAMenuItem4, GBAMenuItem5, GBAMenuItem6};

// Rom menu
static const char GBARomItem1[] = "1MB";
static const char GBARomItem2[] = "2MB";
static const char GBARomItem3[] = "4MB";
static const char GBARomItem4[] = "8MB";
static const char GBARomItem5[] = "16MB";
static const char GBARomItem6[] = "32MB";
static const char* const romOptionsGBA[] = {GBARomItem1, GBARomItem2, GBARomItem3, GBARomItem4, GBARomItem5, GBARomItem6};

// Save menu
static const char GBASaveItem1[] = "4K EEPROM";
static const char GBASaveItem2[] = "64K EEPROM";
static const char GBASaveItem3[] = "256K SRAM/FRAM";
static const char GBASaveItem4[] = "512K SRAM/FRAM";
static const char GBASaveItem5[] = "512K FLASHROM";
static const char GBASaveItem6[] = "1M FLASHROM";
static const char* const saveOptionsGBA[] = {GBASaveItem1, GBASaveItem2, GBASaveItem3, GBASaveItem4, GBASaveItem5, GBASaveItem6};

uint8_t gbaMenu() {
  // create menu with title and 4 options to choose from

  uint8_t bret = 0;
  unsigned char retMenu = questionBox_OLED("GBA Cart Reader", menuOptionsGBA, 6, 1, 1);
  char tmsg[32] = {0};

  // wait for user choice to come back from the question box menu
  switch (retMenu)
  {
    case MENU_CANCEL:
      {
        bret = 1;
      }
      break;

   case MENU_1:
       
      flashRepro_GBA();
      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();
      ResetSystem();
      break;
    case MENU_2:
      // Read rom
      switch (cartSize)
      {
        case 0:
          // create submenu with title and 4 options to choose from
          {
            unsigned char GBARomMenu = questionBox_OLED("Select ROM size", romOptionsGBA, 6, 1, 1);
            // wait for user choice to come back from the question box menu
            switch (GBARomMenu)
            {
              case 1:
                // 1MB
                cartSize = 0x100000;
                break;

              case 2:
                // 2MB
                cartSize = 0x200000;
                break;

              case 3:
                // 4MB
                cartSize = 0x400000;
                break;

              case 4:
                // 8MB
                cartSize = 0x800000;
                break;

              case 5:
                // 16MB
                cartSize = 0x1000000;
                break;

              case 6:
                // 32MB
                cartSize = 0x2000000;
                break;
            }
          }
          break;

        case 1:
          // 1MB
          cartSize = 0x100000;
          break;

        case 4:
          // 4MB
          cartSize = 0x400000;
          break;

        case 8:
          // 8MB
          cartSize = 0x800000;
          break;

        case 16:
          // 16MB
          cartSize = 0x1000000;
          break;

        case 32:
          // 32MB
          cartSize = 0x2000000;
          break;
      }
      OledClear();
      // Change working dir to root
      readROM_GBA();
      compare_checksum_GBA();
      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();
      break;


    case MENU_3:
      // Read save
      if (saveType == 0) 
      {
        // create submenu with title and 6 options to choose from
        unsigned char GBASaveMenu = questionBox_OLED("Select save type:", saveOptionsGBA, 6, 1, 1);
        // wait for user choice to come back from the question box menu
        switch (GBASaveMenu)
        {
          case 1:
            // 4K EEPROM
            saveType = 1;
            break;

          case 2:
            // 64K EEPROM
            saveType = 2;
            break;

          case 3:
            // 256K SRAM/FRAM
            saveType = 3;
            break;

          case 4:
            // 512K SRAM/FRAM
            saveType = 6;
            break;

          case 5:
            // 512K FLASH
            saveType = 4;
            break;

          case 6:
            // 1024K FLASH
            saveType = 5;
            break;
        }
      }

      OledClear();
      switch (saveType)
      {
        case 1:
          // 4K EEPROM
          readEeprom_GBA(4);
          setROM_GBA();
          break;

        case 2:
          // 64K EEPROM
          readEeprom_GBA(64);
          setROM_GBA();
          break;

        case 3:
          // 256K SRAM/FRAM
          readSRAM_GBA(1, 32768, 0);
          setROM_GBA();
          break;

        case 4:
          // 512K FLASH
          readFLASH_GBA(1, 65536, 0);
          setROM_GBA();
          break;

        case 5:
          // 1024K FLASH (divided into two banks)
          switchBank_GBA(0x0);
          setROM_GBA();
          readFLASH_GBA(1, 65536, 0);
          switchBank_GBA(0x1);
          setROM_GBA();
          readFLASH_GBA(0, 65536, 65536);
          setROM_GBA();
          break;

        case 6:
          // 512K SRAM/FRAM
          readSRAM_GBA(1, 65536, 0);
          setROM_GBA();
          break;
      }
      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();
      break;

    case MENU_4:
      // Write save
      if (saveType == 0) 
      {
        // create submenu with title and 6 options to choose from
        unsigned char GBASavesMenu = questionBox_OLED("Select save type:", saveOptionsGBA, 6, 1, 1);
        // wait for user choice to come back from the question box menu
        switch (GBASavesMenu)
        {
          case 1:
            // 4K EEPROM
            saveType = 1;
            break;

          case 2:
            // 64K EEPROM
            saveType = 2;
            break;

          case 3:
            // 256K SRAM/FRAM
            saveType = 3;
            break;

          case 4:
            // 512K SRAM/FRAM
            saveType = 6;
            break;

          case 5:
            // 512K FLASH
            saveType = 4;
            break;

          case 6:
            // 1024K FLASH
            saveType = 5;
            break;
        }
      }

      OledClear();
      switch (saveType)
      {
        case 1:
          // 4K EEPROM
          writeEeprom_GBA(4);
          writeErrors = verifyEEP_GBA(4);
          if (writeErrors == 0) 
          {
            OledShowString(0,4,"Verified OK",8);            
          }
          else 
          {
            //
            sprintf(tmsg,"Error: %d bytes.",writeErrors);
            OledShowString(0,4,tmsg,8);
            print_Error("Did not verify...", false);
          }
          setROM_GBA();
          break;

        case 2:
          // 64K EEPROM
          writeEeprom_GBA(64);
          writeErrors = verifyEEP_GBA(64);
          if (writeErrors == 0) 
          {
            OledShowString(0,4,"Verified OK",8);            
          }
          else 
          {
            //
            sprintf(tmsg,"Error: %d bytes.",writeErrors);
            OledShowString(0,4,tmsg,8);
            print_Error("Did not verify...", false);
          }
          setROM_GBA();
          break;

        case 3:
          // 256K SRAM/FRAM
          writeSRAM_GBA(1, 32768, 0);
          writeErrors = verifySRAM_GBA(32768, 0);
          if (writeErrors == 0) 
          {
            OledShowString(0,4,"Verified OK",8);            
          }
          else 
          {
            //
            sprintf(tmsg,"Error: %d bytes.",writeErrors);
            OledShowString(0,4,tmsg,8);
            print_Error("Did not verify...", false);
          }
          setROM_GBA();
          break;

        case 4:
          // 512K FLASH
          idFlash_GBA();
          resetFLASH_GBA();
          if (strcmp(flashid, "BFD4") != 0) 
          {
            OledShowString(0,3,"Flashrom Type \n not supported!",8);
            sprintf(tmsg,"ID: %s",flashid);
            print_Error(tmsg, true);
          }
          eraseFLASH_GBA();
          if (blankcheckFLASH_GBA(65536)) 
          {
            writeFLASH_GBA(1, 65536, 0);
            verifyFLASH_GBA(65536, 0);
          }
          else 
          {
            print_Error("Erase failed!", false);
          }
          setROM_GBA();
          break;

        case 5:
          // 1M FLASH
          idFlash_GBA();
          resetFLASH_GBA();
          if (strcmp(flashid, "C209") != 0) 
          {
            OledShowString(0,3,"Flashrom Type \n not supported!",8);
            sprintf(tmsg,"ID: %s",flashid);
            print_Error(tmsg, true);
          }
          eraseFLASH_GBA();
          // 131072 bytes are divided into two 65536 byte banks
          switchBank_GBA(0x0);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536))
          {
            writeFLASH_GBA(1, 65536, 0);
            verifyFLASH_GBA(65536, 0);
          }
          else 
          {
            print_Error("Erase failed!", false);
          }
          switchBank_GBA(0x1);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536)) 
          {
            writeFLASH_GBA(0, 65536, 65536);
            verifyFLASH_GBA(65536, 65536);
          }
          else 
          {
            print_Error("Erase failed!", false);
          }
          setROM_GBA();
          break;

        case 6:
          // 512K SRAM/FRAM
          writeSRAM_GBA(1, 65536, 0);
          writeErrors = verifySRAM_GBA(65536, 0);
          if (writeErrors == 0) 
          {
            OledShowString(0,7,"Verified OK",8);            
          }
          else 
          {
            //
            sprintf(tmsg,"Error: %d bytes.",writeErrors);
            OledShowString(0,5,tmsg,8);
            print_Error("Did not verify...", false);
          }
          setROM_GBA();
          break;
      }
      OledShowString(0,7,"Press OK Button...",8);
      WaitOKBtn();
      break;
      
    case MENU_5:
      {
       
        // create submenu with title and 7 options to choose from
        unsigned char GBASaveMenu = questionBox_OLED("Select save type:", saveOptionsGBA, 6, 1, 1);

        // wait for user choice to come back from the question box menu
        switch (GBASaveMenu)
        {
          case 0:
            // 4K EEPROM
            saveType = 1;
            break;

          case 1:
            // 64K EEPROM
            saveType = 2;
            break;

          case 2:
            // 256K SRAM/FRAM
            saveType = 3;
            break;

          case 3:
            // 512K SRAM/FRAM
            saveType = 6;
            break;

          case 4:
            // 512K FLASH
            saveType = 4;
            break;

          case 5:
            // 1024K FLASH
            saveType = 5;
            break;
        }
      }
       
      break;


    case MENU_6:
      ResetSystem();
      break;
  }

  return  bret;
}



void gbaScreen()
{
  //
  while(1)
  {
    //
    setup_GBA();
    if(gbaMenu() > 0)
    {
      //
      break;
    }
  }
}




//******************************************
// End of File
//******************************************