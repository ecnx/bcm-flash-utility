/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Shared Project Header
 * ------------------------------------------------------------------ */

#include "config.h"
#include "firmware.h"
#include "board_api.h"

#ifndef BRFW_H
#define BRFW_H

#define BRFW_VERSION "1.1.34"

#define FLASH_NO_PROMPT 1

/**
 * Print board details
 */
extern void print_board_info ( int boardfd );

/**
 * Turn on or off single led
 */
extern int set_led_state ( int boardfd, unsigned int num, int val );

/**
 * Blink all leds found on the board n times
 */
extern void blink_leds ( int boardfd, unsigned int count );

/**
 * Perform mips soft reset of system
 */
extern int mips_soft_reset ( int boardfd );

/**
 * Flash firmware file (CFE, kernel and rootfs)
 */
extern int flash_firmware ( int boardfs, const char *path, int flags );

/**
 * Verify Broadcom firmware file
 */
extern int verify_bcm_firmware ( int boardfd, const unsigned char *image, unsigned int length );

/**
 * Check if bootloader is present in firmware
 */
extern int firmware_loader_is_present ( const unsigned char *image, unsigned int length,
    int *result );

/**
 * Flash Broadcom firmware file
 */
extern int flash_bcm_firmware ( int boardfd, unsigned char *image, unsigned int length );

/**
 * Calculate crc32 checksum of buffer
 */
extern unsigned int crc32buf ( const unsigned char *buf, unsigned int len );


#endif
