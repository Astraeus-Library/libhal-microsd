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

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                              d                        */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv) {
    if(pdrv == 0) {
        return RES_OK;
    }
    return STA_NOINIT;
}




/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv) {
    if(pdrv == 0) {
        // Assuming you instantiate the SD card somewhere globally.
        if(sd.init() == hal::success()) {
            return RES_OK;
        }
    }
    return STA_NOINIT;
}




/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
    if(pdrv == 0) {
        while(count--) {
            auto result = sd.read_block(sector, buff);
            if(result != hal::success()) {
                return RES_ERROR;
            }
            buff += 512;  // Increase buffer pointer.
            sector++;
        }
        return RES_OK;
    }
    return RES_PARERR;
}




/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
    if(pdrv == 0) {
        while(count--) {
            if(sd.write_block(sector, buff) != hal::success()) {
                return RES_ERROR;
            }
            buff += 512;
            sector++;
        }
        return RES_OK;
    }
    return RES_PARERR;
}


#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff) {
    if(pdrv == 0) {
        switch(cmd) {
            case CTRL_SYNC:
                return RES_OK; 
            case GET_SECTOR_COUNT:
                // This can be improved using card capacity commands
                *(DWORD*)buff = 0x1000000; // Placeholder
                return RES_OK;
            case GET_BLOCK_SIZE:
                *(DWORD*)buff = 512; // SD card's block size is 512 bytes
                return RES_OK;
            default:
                return RES_PARERR;
        }
    }
    return RES_PARERR;
}


