/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Firmware Header
 * ------------------------------------------------------------------ */

#include <stdint.h>

#ifndef FIRMWARE_H
#define FIRMWARE_H

/**
 * Broadcom firmware header structure
 * (based on openwrt docs and http://skaya.enix.org/wiki/FirmwareFormat)
 */

struct bcm_header_t
{
    uint8_t magic[4];           /* 36 00 00 00 in all my firmwares (it might rather be a version number) */
    uint8_t vendor[20];         /* generally "Broadcom Corporation" (but some vendors did set this string to spaces) */
    uint8_t version[14];        /* generally "Firmware version 1.0" (sometimes erased, too) */
    uint8_t chip_id[6];         /* router chip id */
    uint8_t board_id[16];       /* router board id */
    uint8_t endian_flag[2];     /* "1" each time (that is, 31 00) */

    uint8_t total_size[10];     /* the size of the image, without the header (that is, file size minus 256) */
    uint8_t loader_addr[12];    /* the address at which the loader should be flashed (0xBFC00000?) */
    uint8_t loader_size[10];    /* the size of the loader */
    uint8_t rootfs_addr[12];    /* the address at which the root filesystem should be flashed (was always 0xBFC10100) */
    uint8_t rootfs_size[10];    /* the size of the root filesystem */
    uint8_t kernel_addr[12];    /* the address at which the kernel filesystem should be flashed (was always root filesystem address + root filesystem size) */
    uint8_t kernel_size[10];    /* the size of the kernel */

    uint8_t reserved_1[78];     /* reserved for future use #1 */
    uint32_t data_crc32;        /* the data checksum (from byte 256 to the end of the file) */
    uint32_t rootfs_crc32;      /* the rootfs checksum */
    uint32_t kernel_crc32;      /* the kernel checksum */
    uint32_t sequence;          /* image sequence number */
    uint32_t root_length;       /* set root_length to 0 to satisfy devices that check CRC on every boot */
    uint32_t header_crc32;      /* the header checksum (from byte 0 to byte 235) */
    uint8_t reserved_2[16];     /* reserved for future use #2 */
};

#endif
