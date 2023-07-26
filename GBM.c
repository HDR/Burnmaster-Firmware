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





/**********************
  LOW LEVEL
**********************/
void delay_GBM()
{
  //
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}


// Read one word out of the cartridge
byte readByte_GBM(word myAddress) 
{
  // Set data pins to Input
  GPIO_CTL1(DATA) = 0x44444444;
  GPIO_OCTL(ADDRLOW) = (GPIO_OCTL(ADDRLOW)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDRHIGH) = (GPIO_OCTL(ADDRHIGH)&0xFFFFF0FF) + (myAddress & 0x0F00);

  
  delay_GBM();

  // Switch CS(PH3) and RD(PH6) to LOW
  gpio_bit_reset(CTRL,CS);
  gpio_bit_reset(CTRL,RD);

  delay_GBM();

  // Read
  byte tempByte = GPIO_ISTAT(DATA) >> 8;

  // Switch CS(PH3) and RD(PH6) to HIGH
  gpio_bit_set(CTRL,RD);
  gpio_bit_set(CTRL,CS);

  return tempByte;
}

// Write one word to data pins of the cartridge
void writeByte_GBM(word myAddress, byte myData) 
{
  // Set data pins to Output
  GPIO_CTL1(DATA) = 0x33333333;

  GPIO_OCTL(ADDRLOW) = (GPIO_OCTL(ADDRLOW)&0xFFFF000F) + ((myAddress << 8)&0xFF00) + ((myAddress >> 8)&0xF0);
  GPIO_OCTL(ADDRHIGH) = (GPIO_OCTL(ADDRHIGH)&0xFFFFF0FF) + (myAddress & 0x0F00);
  GPIO_OCTL(DATA) = (GPIO_OCTL(DATA)&0xFFFF00FF) + ((myData << 8)&0xFF00);

  // Pull CS(PH3) and write(PH5) low
  gpio_bit_reset(CTRL,CS);
  gpio_bit_reset(CTRL,WR);

  delay_GBM();

  // Pull CS(PH3) and write(PH5) high
  gpio_bit_set(CTRL,WR);
  gpio_bit_set(CTRL,CS);

  delay_GBM();

  // Set data pins to Input (or read errors??!)
  GPIO_CTL1(DATA) = 0x44444444;
}

/**********************
  HELPER FUNCTIONS
**********************/
void printSdBuffer(word startByte, word numBytes) 
{
  char tmsg[30] = {0};
  for (int i = 0; i < numBytes; i++) 
  {
    for (byte c = 0; c < 10; c++) 
    {
      // Convert to char array so we don't lose leading zeros
      sprintf(tmsg,"%s%02X",tmsg,sdBuffer[startByte + i*10 + c]);
    }
    // Add a new line every 10 bytes
    OledShowString(0,i,tmsg,8);
  }
}

void readROM_GBM(word numBanks) 
{
  OledShowString(0,0,"Reading Rom...",8);   

  // Get name, add extension and convert to char array for sd lib
  foldern = load_dword();
  //
  sprintf(fileName, "GBM%d", foldern);
  strcat(fileName, ".bin");
  my_mkdir("/NP");
  f_chdir("/NP");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  save_dword(foldern);

  FIL tf;
  // Open file on sd card
  if (f_open(&tf, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) 
  {
    print_Error("Can't create file on SD!", true);
  }
  else 
  {
    // Read rom
    word currAddress = 0;

    for (word currBank = 1; currBank < numBanks; currBank++) 
    {
      // Set rom bank
      writeByte_GBM(0x2100, currBank);

      // Switch bank start address
      if (currBank > 1) {
        currAddress = 0x4000;
      }

      for (; currAddress < 0x7FFF; currAddress += 512) 
      {
        for (int currByte = 0; currByte < 512; currByte++) {
          sdBuffer[currByte] = readByte_GBM(currAddress + currByte);
        }
        UINT wdt;
        f_write(&tf, sdBuffer, 512, &wdt);
      }
    }

    // Close the file:
    f_close(&tf);

    // Signal end of process
    OledShowString(0,1,"Saved to NP/",8);
    OledShowString(72,1,fileName,8);
     
  }
}

/**********************
  GB Memory Functions
**********************/
void send_GBM(byte myCommand) 
{
  switch (myCommand) {
    case 0x01:
      //CMD_01h -> ???
      writeByte_GBM(0x0120, 0x01);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x02:
      //CMD_02h -> Write enable Step 2
      writeByte_GBM(0x0120, 0x02);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x03:
      //CMD_03h -> Undo write Step 2
      writeByte_GBM(0x0120, 0x03);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x04:
      //CMD_04h -> Map entire flashrom (MBC4 mode)
      writeByte_GBM(0x0120, 0x04);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x05:
      //CMD_05h -> Map menu (MBC5 mode)
      writeByte_GBM(0x0120, 0x05);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x08:
      //CMD_08h -> disable writes/reads to/from special Nintendo Power registers (those at 0120h..013Fh)
      writeByte_GBM(0x0120, 0x08);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x09:
      //CMD_09h Wakeup -> re-enable access to ports 0120h..013Fh
      writeByte_GBM(0x0120, 0x09);
      writeByte_GBM(0x0121, 0xAA);
      writeByte_GBM(0x0122, 0x55);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x0A:
      //CMD_0Ah -> Write enable Step 1
      writeByte_GBM(0x0120, 0x0A);
      writeByte_GBM(0x0125, 0x62);
      writeByte_GBM(0x0126, 0x04);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x10:
      //CMD_10h -> disable writes to normal MBC registers (such like 2100h)
      writeByte_GBM(0x0120, 0x10);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x11:
      //CMD_11h -> re-enable access to MBC registers like 0x2100
      writeByte_GBM(0x0120, 0x11);
      writeByte_GBM(0x013F, 0xA5);
      break;

    default:
      print_Error("Unknown Command!", true);
      break;
  }
}

void send_GBM1(byte myCommand, word myAddress, byte myData) 
{
  byte myAddrLow = myAddress & 0xFF;
  byte myAddrHigh = (myAddress >> 8) & 0xFF;

  switch (myCommand) {
    case 0x0F:
      // CMD_0Fh -> Write address/byte to flash
      writeByte_GBM(0x0120, 0x0F);
      writeByte_GBM(0x0125, myAddrHigh);
      writeByte_GBM(0x0126, myAddrLow);
      writeByte_GBM(0x0127, myData);
      writeByte_GBM(0x013F, 0xA5);
      break;

    default:
      print_Error("Unknown Command!", true);
      break;
  }
}

void switchGame_GBM(byte myData) 
{
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  //CMD_C0h -> map selected game without reset
  writeByte_GBM(0x0120, 0xC0 & myData);
  writeByte_GBM(0x013F, 0xA5);
}

void resetFlash_GBM() {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  // Send reset command
  writeByte_GBM(0x2100, 0x01);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0xF0);
  delay(100);
}

boolean readFlashID_GBM() 
{
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  writeByte_GBM(0x2100, 0x01);
  // Read ID command
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x90);

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_GBM(0), readByte_GBM(1));
  //
  OledShowString(0,1,"Flash ID: ",8);
  OledShowString(60,1,flashid,8);
  //
  if (strcmp(flashid, "C289") == 0) 
  {
    resetFlash_GBM();
    return true;
  }
  else 
  {
    print_Error("Unknown Flash ID!", true);
    return false;
  }
}

void eraseFlash_GBM() 
{
  OledShowString(0,0,"Erasing...",8);
   

  //enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Unprotect sector 0
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x60);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x40);

  // Wait for unprotect to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Send erase command
  send_GBM1(0x0F, 0x5555, 0xaa);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x80);
  send_GBM1(0x0F, 0x5555, 0xaa);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x10);

  // Wait for erase to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Reset flashrom
  resetFlash_GBM();
}

boolean blankcheckFlash_GBM() 
{
  OledShowString(0,0,"Blankcheck...",8);
   

  //enable access to ports 0120h (F2)
  send_GBM(0x09);

  // Map entire flashrom
  send_GBM(0x04);
  // Disable ports 0x0120...
  send_GBM(0x08);

  // Read rom
  word currAddress = 0;

  for (byte currBank = 1; currBank < 64; currBank++) {
    // Set rom bank
    writeByte_GBM(0x2100, currBank);

    // Switch bank start address
    if (currBank > 1) {
      currAddress = 0x4000;
    }

    for (; currAddress < 0x7FFF; currAddress++) {
      if (readByte_GBM(currAddress) != 0xFF) {
        return 0;
      }
    }
  }
  return 1;
}

void writeFlash_GBM() 
{
  OledShowString(0,0,"Writing...",8);   

   FIL tf;
   uint32_t fileSize = 0;
  // Open file on sd card
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {
    // Get rom size from file
    fileSize = f_size(&tf);
    fileSize = fileSize / 0x4000;
    if (fileSize > 64) 
    {
      f_close(&tf);
      print_Error("File is too big.", true);
    }

    // Enable access to ports 0120h
    send_GBM(0x09);
    // Enable write
    send_GBM(0x0A);
    send_GBM(0x2);

    // Map entire flash rom
    send_GBM(0x4);

    // Set bank for unprotect command, writes to 0x5555 need odd bank number
    writeByte_GBM(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Unprotect sector 0
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0x60);
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0x40);

    // Check if flashrom is ready for writing or busy
    while ((readByte_GBM(0) & 0x80) != 0x80) {}

    // first bank: 0x0000-0x7FFF,
    word currAddress = 0x0;

    // Write 63 banks
    for (byte currBank = 0x1; currBank < fileSize; currBank++) 
    {
      // Blink led
      LED_BLINK(LED1);
      showPersent(currBank - 1,fileSize,60,0);

      // all following banks: 0x4000-0x7FFF
      if (currBank > 1) {
        currAddress = 0x4000;
      }

      // Write single bank in 128 byte steps
      for (; currAddress < 0x7FFF; currAddress += 128) 
      {
        // Fill SD buffer
        UINT rdt;
        f_read(&tf, sdBuffer, 128, &rdt);

        // Enable access to ports 0x120 and 0x2100
        send_GBM(0x09);
        send_GBM(0x11);

        // Set bank
        writeByte_GBM(0x2100, 0x1);

        // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
        send_GBM(0x10);
        send_GBM(0x08);

        // Write flash buffer command
        writeByte_GBM(0x5555, 0xAA);
        writeByte_GBM(0x2AAA, 0x55);
        writeByte_GBM(0x5555, 0xA0);

        // Wait until flashrom is ready again
        while ((readByte_GBM(0) & 0x80) != 0x80) {}

        // Enable access to ports 0x120 and 0x2100
        send_GBM(0x09);
        send_GBM(0x11);

        // Set bank
        writeByte_GBM(0x2100, currBank);

        // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
        send_GBM(0x10);
        send_GBM(0x08);

        // Fill flash buffer
        for (word currByte = 0; currByte < 128; currByte++) {
          writeByte_GBM(currAddress + currByte, sdBuffer[currByte]);
        }
        // Execute write
        writeByte_GBM(currAddress + 127, 0xFF);

        // Wait for write to complete
        while ((readByte_GBM(currAddress) & 0x80) != 0x80) {}
      }
    }

    showPersent(1,1,60,0);
    // Close the file:
    f_close(&tf);
  }
  else 
  {
    print_Error("Can't open file!", false);
  }
}

void readMapping_GBM() 
{
  // Enable ports 0x0120
  send_GBM(0x09);

  // Set WE and WP
  send_GBM(0x0A);
  send_GBM(0x2);

  // Enable hidden mapping area
  writeByte_GBM(0x2100, 0x01);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x77);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x77);

  // Read mapping
  OledShowString(0,0,"Read Mapping...",8);
   

  // Get name, add extension and convert to char array for sd lib
  foldern = load_dword();
  //
  sprintf(fileName, "GBM%d", foldern);
  strcat(fileName, ".map");
  my_mkdir("/NP");
  f_chdir("NP");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  save_dword(foldern);


  FIL tf;
  // Open file on sd card
  if (f_open(&tf, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) 
  {
    print_Error("Can't create file on SD!", true);
  }
  else 
  {
    for (byte currByte = 0; currByte < 128; currByte++) 
    {
      sdBuffer[currByte] = readByte_GBM(currByte);
    }
    UINT wdt;
    f_write(&tf, sdBuffer, 128, &wdt);

    // Close the file:
    f_close(&tf);

    // Signal end of process
    printSdBuffer(0, 20);
    printSdBuffer(102, 20);

    OledShowString(0,5,"Saved to NP/",8);
    OledShowString(72,5,fileName,8);
     
  }

  // Reset flash to leave hidden mapping area
  resetFlash_GBM();
}

void eraseMapping_GBM() 
{
  OledShowString(0,0,"Erasing...",8);   

  //enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Unprotect sector 0
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x60);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x40);

  // Wait for unprotect to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Send erase command
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x60);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x04);

  // Wait for erase to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Reset flashrom
  resetFlash_GBM();
}

boolean blankcheckMapping_GBM() 
{
  OledShowString(0,0,"Blankcheck...",8);
   

  // Enable ports 0x0120
  send_GBM(0x09);

  // Set WE and WP
  send_GBM(0x0A);
  send_GBM(0x2);

  // Enable hidden mapping area
  writeByte_GBM(0x2100, 0x01);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x77);
  send_GBM1(0x0F, 0x5555, 0xAA);
  send_GBM1(0x0F, 0x2AAA, 0x55);
  send_GBM1(0x0F, 0x5555, 0x77);

  // Disable ports 0x0120...
  send_GBM(0x08);

  // Read rom
  for (byte currByte = 0; currByte < 128; currByte++) 
  {
    if (readByte_GBM(currByte) != 0xFF) 
    {
      return 0;
    }
  }
  return 1;
}

void writeMapping_GBM() 
{
  OledShowString(0,0,"Writing...",8);
   
  FIL tf;
  // Open file on sd card
  if (f_open(&tf, filePath, FA_READ) == FR_OK) 
  {
    // Get map file size and check if it exceeds 128KByte
    if (f_size(&tf) > 0x80) 
    {
      f_close(&tf);
      print_Error("File is too big.", true);
    }

    // Enable access to ports 0120h
    send_GBM(0x09);

    // Enable write
    send_GBM(0x0A);
    send_GBM(0x2);

    // Map entire flash rom
    send_GBM(0x4);

    // Set bank, writes to 0x5555 need odd bank number
    writeByte_GBM(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Unlock write to map area
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0x60);
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0xE0);

    // Check if flashrom is ready for writing or busy
    while ((readByte_GBM(0) & 0x80) != 0x80) {}

    // Fill SD buffer
    UINT rdt;
    f_read(&tf, sdBuffer, 128, &rdt);

    // Enable access to ports 0x120 and 0x2100
    send_GBM(0x09);
    send_GBM(0x11);

    // Set bank
    writeByte_GBM(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Write flash buffer command
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0xA0);

    // Wait until flashrom is ready again
    while ((readByte_GBM(0) & 0x80) != 0x80) {}

    // Enable access to ports 0x120 and 0x2100
    send_GBM(0x09);
    send_GBM(0x11);

    // Set bank
    writeByte_GBM(0x2100, 0);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Fill flash buffer
    for (word currByte = 0; currByte < 128; currByte++) 
    {
      // Blink led
      LED_BLINK(LED1);
      writeByte_GBM(currByte, sdBuffer[currByte]);
    }
    // Execute write
    writeByte_GBM(127, 0xFF);

    // Close the file:
    f_close(&tf);
    OledShowString(20,1,"Done",8);
  }
  else 
  {
    print_Error("Can't open file!", false);
  }
}


/******************************************
  Setup
*****************************************/
void setup_GBM() 
{
  //
  OledClear();
  // Set RST(PH0) to Input
  // Activate Internal Pullup Resistors
  gpio_init(CTRL,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,RST);

  // Set Address Pins to Output
  // AD0-AD7
  gpio_init(ADDRLOW,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(4,15));
  // AD8-AD15
  gpio_init(ADDRHIGH,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(8,11));

  // Set Control Pins to Output CS(PH3) WR(PH5) RD(PH6)
  gpio_init(CTRL,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,CS|WR|RD);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  gpio_bit_set(CTRL,CS|WR|RD);

  // Set Data Pins (D0-D7) to Input
  gpio_init(DATA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,BITS(8,15));

  delay(400);

  // Check for Nintendo Power GB Memory cart
  byte timeout = 0;

  // First byte of NP register is always 0x21
  while (readByte_GBM(0x120) != 0x21) {
    // Enable ports 0x120h (F2)
    send_GBM(0x09);
    delay_GBM();
    //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    timeout++;
    if (timeout > 10) 
    {
      OledShowString(0,0,"Error: Time Out!",8);
      print_Error("Please power cycle!", true);
    }
  }
}


/******************************************
  Menu
*****************************************/
// GBM menu items
static const char gbmMenuItem1[] = "Read ID";
static const char gbmMenuItem2[] = "Read Flash";
static const char gbmMenuItem3[] = "Erase Flash";
static const char gbmMenuItem4[] = "Blankcheck";
static const char gbmMenuItem5[] = "Write Flash";
static const char gbmMenuItem6[] = "Read Mapping";
static const char gbmMenuItem7[] = "Write Mapping";
static const char* const menuOptionsGBM[] = {gbmMenuItem1, gbmMenuItem2, gbmMenuItem3, gbmMenuItem4, gbmMenuItem5, gbmMenuItem6, gbmMenuItem7};

uint8_t gbmMenu() 
{
  // create menu with title and 7 options to choose from
  uint8_t bret = 0;
  unsigned char gbmMenu = questionBox_OLED("GB Memory Menu -", menuOptionsGBM, 7, 1, 1);

  // wait for user choice to come back from the question box menu
  switch (gbmMenu)
  {
    // Read Flash ID
    case MENU_CANCEL:
      bret = 1;
      break;
    case MENU_1:
      // Clear screen
      //OledClear();
      readFlashID_GBM();
      break;

    // Read Flash
    case MENU_2:
      // Clear screen
      //OledClear();
      // Print warning
      OledShowString(0,0,"Attention!\nAlways power cycle\ncartreader directly\nbefore reading\n\nPress OK Button\nto continue",8);
      WaitOKBtn();
      // Clear screen
      OledClear();
      // Enable access to ports 0120h
      send_GBM(0x09);
      // Map entire flashrom
      send_GBM(0x04);
      // Disable ports 0x0120...
      send_GBM(0x08);
      // Read 1MB rom
      readROM_GBM(64);
      break;

    // Erase Flash
    case MENU_3:
      // Clear screen
      //OledClear();
      // Print warning
      OledShowString(0,0,"Attention!\nThis will erase your\nNP Cartridge.\n\n\nPress OK Button\nto continue",8);
      WaitOKBtn();
      // Clear screen
      OledClear();
      eraseFlash_GBM();
      break;

    // Blankcheck Flash
    case MENU_4:
      // Clear screen
      //OledClear();
      if (blankcheckFlash_GBM()) 
      {
        OledShowString(20,1,"OK",8);
         
      }
      else 
      {
        OledShowString(20,1,"ERROR!",8);         
      }
      break;

    // Write Flash
    case MENU_5:
      // Clear screen
      //OledClear();
      filePath[0] = '\0';
      // Launch file browser
      fileBrowser("/","Select 1MB file:");
      OledClear();
      // Write rom
      writeFlash_GBM();
      break;

    // Read mapping
    case MENU_6:
      // Clear screen
      //OledClear();
      // Read mapping
      readMapping_GBM();
      break;

    // Write mapping
    case MENU_7:
      // Clear screen
      //OledClear();

      // Print warning
      OledShowString(0,0,"Attention!\nThis will erase your\nNP Cartridge's\nmapping data\n\nPress OK Button\nto continue",8);
      WaitOKBtn();
      // Clear screen
      OledClear();
      // Clear filepath
      filePath[0] = '\0';
      // Launch file browser
      fileBrowser("/","Select MAP file");
      OledClear();
      // Clear screen
      OledClear();

      // Erase mapping
      eraseMapping_GBM();
      if (blankcheckMapping_GBM()) 
      {
        OledShowString(20,1,"OK",8);         
      }
      else 
      {
        print_Error("Erasing failed!", false);
        break;
      }

      // Write mapping
      writeMapping_GBM();
      break;
  }

  if(bret == 0)
  {
    OledShowString(0,7,"Press OK Button...",8);
    WaitOKBtn();
  }
  return bret;
}


void gbmScreen()
{
  //
  while(1)
  {
    //
    setup_GBM();
    if(gbmMenu() > 0)
    {
      //
      break;
    }
  }
}