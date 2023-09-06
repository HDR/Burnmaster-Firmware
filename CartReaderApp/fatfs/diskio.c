/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include <gd32f10x_sdio.h>
#include "sdcard.h"
#include "math.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		1	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */



DWORD get_fattime(void) {
	/* 返回当前时间戳 */
	return	  ((DWORD)(2022 - 1980) << 25)	/* Year 2021 */
			| ((DWORD)1 << 21)				/* Month 1 */
			| ((DWORD)1 << 16)				/* Mday 1 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				  /* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
}


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static volatile DSTATUS currStat = STA_NOINIT;

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();
		// translate the reslut code here
		break;
	case DEV_MMC :
		stat = currStat;
		// translate the reslut code here
		break;
	case DEV_USB :
		//result = USB_disk_status();
		// translate the reslut code here
		break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_initialize();
		// translate the reslut code here
		break;
	case DEV_MMC :
                if(SD_Status == SD_OK)
		stat = STA_OK;
                currStat = stat; 
		break;
	case DEV_USB :
		//result = USB_disk_initialize();
		// translate the reslut code here
		break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;
	sd_error_enum SD_state;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here
		//result = RAM_disk_read(buff, sector, count);
		// translate the reslut code here
		break;
	case DEV_MMC :
		if(count ==  1){
			SD_state = sd_block_read((uint32_t *)buff, sector*SD_BLOCKSIZE, SD_BLOCKSIZE);
		}else{
			SD_state=sd_multiblocks_read((uint32_t *)buff,sector*SD_BLOCKSIZE,SD_BLOCKSIZE,count);
		}

		if(SD_state==SD_OK)
		{
			/* Check if the Transfer is finished */
			//SD_state=SD_WaitReadOperation();
			
			/* Wait until end of DMA transfer */
			while(sd_transfer_state_get() != SD_NO_TRANSFER){};
                        res = RES_OK;
		}			
		else
			res = RES_PARERR;

		break;
	case DEV_USB :
		// translate the arguments here
		//result = USB_disk_read(buff, sector, count);
		// translate the reslut code here
		break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	sd_error_enum SD_state;


	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here
                break;
	case DEV_MMC :
		// translate the arguments here
		if(count ==  1){
			SD_state = sd_block_write((uint32_t *)buff, sector*SD_BLOCKSIZE, SD_BLOCKSIZE);
		}else{
			SD_state=sd_multiblocks_write((uint32_t *)buff,sector*SD_BLOCKSIZE,SD_BLOCKSIZE,count);
		}

		if(SD_state==SD_OK)
		{
			/* Check if the Transfer is finished */
			//SD_state=SD_WaitReadOperation();
			
			/* Wait until end of DMA transfer */
			while(sd_transfer_state_get() != SD_NO_TRANSFER){};
                        res = RES_OK;
		}			
		else
			res = RES_PARERR;
                break;

	case DEV_USB :
		// translate the arguments here
                break;
	}

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;
	int result;

	switch (pdrv) {
	case DEV_RAM :
                break;
	case DEV_MMC :
		// Process of the command for the MMC/SD card
                switch (cmd) 
                {
                        // Get R/W sector size (WORD) 
                        case GET_SECTOR_SIZE :    
                                *(WORD * )buff = sd_cardinfo.card_blocksize;
                        break;
                        // Get erase block size in unit of sector (DWORD)
                        case GET_BLOCK_SIZE :      
                                *(DWORD * )buff = sd_cardinfo.card_blocksize;
                        break;

                        case GET_SECTOR_COUNT:
                                *(DWORD * )buff = sd_cardinfo.card_capacity/sd_cardinfo.card_blocksize;
                                break;
                        case CTRL_SYNC :
                        break;
                }
                res = RES_OK;
                break;
	case DEV_USB :
                break;
	}
	return res;
}

