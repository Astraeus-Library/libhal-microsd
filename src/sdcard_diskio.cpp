#include "diskio.h"  // Include FatFS disk I/O interface
#include "libhal-sd/microsd.hpp"

using namespace hal::sd;

extern microsd_card sd; // Assuming you've created a global instance of your microsd_card class somewhere

// Implement disk status function
DSTATUS disk_status(BYTE pdrv) {
    if (pdrv == 0) {
        // Assuming pdrv 0 corresponds to the SD card
        // Implement status check logic or simply return 0 if the status can't be checked
        return 0; // 0 means OK, no errors
    }
    return STA_NOINIT; // Drive not initialized
}

// Implement disk initialization function
DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv == 0) {
        // Initialize your SD card and return the status
        auto result = sd.init();
        return result == hal::success() ? 0 : STA_NOINIT;
    }
    return STA_NOINIT;
}

// Implement disk read function
DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    if (pdrv == 0) {
        for (UINT i = 0; i < count; i++) {
            auto result = sd.read_block(sector + i, buff);
            if (!result) {
                return RES_ERROR;
            }
            buff += 512; // Move to the next block
        }
        return RES_OK;
    }
    return RES_PARERR;
}

// Implement disk write function
DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
    if (pdrv == 0) {
        for (UINT i = 0; i < count; i++) {
            if (sd.write_block(sector + i, buff) != hal::success()) {
                return RES_ERROR;
            }
            buff += 512; // Move to the next block
        }
        return RES_OK;
    }
    return RES_PARERR;
}

// Implement disk I/O control function
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv == 0) {
        switch (cmd) {
            case CTRL_SYNC:
                // Flush the cache if you have caching
                return RES_OK;
            case GET_SECTOR_COUNT: // if needed
                // *(DWORD*)buff = total sectors;
                return RES_OK;
            case GET_SECTOR_SIZE: // if needed
                // *(WORD*)buff = sector size;
                return RES_OK;
            case GET_BLOCK_SIZE: // if needed
                // *(DWORD*)buff = block size;
                return RES_OK;
            default:
                return RES_PARERR;
        }
    }
    return RES_PARERR;
}
