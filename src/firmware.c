/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Firmware Operations
 * ------------------------------------------------------------------ */

#include "brfw.h"

#define SDUMP(N, V) \
    hdr_dump_string(N, V, sizeof(V)); \
    printf("\n");

#define SDUMP2(N, V) \
    hdr_dump_string(N, V, sizeof(V));

/* Copy firmware header string into buffer */
static int hdr_copy_string ( char *dst, unsigned int dst_size, const unsigned char *src,
    unsigned int src_size )
{
    unsigned int i;
    unsigned int len = src_size;

    if ( len >= dst_size )
    {
        return -1;
    }

    for ( i = 0; i < len; i++ )
    {
        dst[i] = src[i];
    }

    dst[i] = '\0';
    return 0;
}

/* Copy firmware header string into buffer */
static void hdr_dump_string ( const char *prefix, const unsigned char *src, unsigned int src_size )
{
    char dst[256];

    if ( hdr_copy_string ( dst, sizeof ( dst ), src, src_size ) >= 0 )
    {
        printf ( "%s: %s", prefix, dst );
    }
}

/**
 * Print hex value padded to eight chars
 */
static void print_pad_hex ( unsigned int value )
{
    ssize_t len;
    char buffer[64];

    snprintf ( buffer, sizeof ( buffer ), "%x", value );
    for ( len = 8 - strlen ( buffer ); len > 0; len-- )
    {
        putchar ( '0' );
    }

    printf ( "%s", buffer );
}

/**
 * Print hex value padded to two chars
 */
static void print_pad_hex_2 ( unsigned int value )
{
    printf ( value <= 0xF ? "0%x" : "%x", value );
}

/**
 * Verify Broadcom firmware file
 */
int verify_bcm_firmware ( int boardfd, const unsigned char *image, unsigned int length )
{
    int result;
    int chip_id;
    unsigned int flash_size;
    unsigned int data_crc32;
    unsigned int rootfs_crc32;
    unsigned int kernel_crc32;
    unsigned int header_crc32;
    unsigned int total_size = 0;
    unsigned int loader_size = 0;
    unsigned int rootfs_size = 0;
    unsigned int kernel_size = 0;
    const struct bcm_header_t *header = ( const struct bcm_header_t * ) image;
    char str[64];
    char board_id[64];

    /* validate firmware magic */
    if ( length < sizeof ( struct bcm_header_t )
        || header->magic[0] != 0x36 || header->magic[1] || header->magic[2] || header->magic[3] )
    {
        fprintf ( stderr, "bcm header not found.\n" );
        return -1;
    }

    /* Show board information */
    print_board_info ( boardfd );

    printf ( "\n------- firmware information -------\n" );

    /* Parse total size */
    if ( hdr_copy_string ( str, sizeof ( str ), header->total_size,
            sizeof ( header->total_size ) ) < 0 || sscanf ( str, "%u", &total_size ) <= 0 )
    {
        fprintf ( stderr, "failed to parse total size.\n" );
        return -1;
    }

    /* Parse loader size */
    if ( hdr_copy_string ( str, sizeof ( str ), header->loader_size,
            sizeof ( header->loader_size ) ) < 0 || sscanf ( str, "%u", &loader_size ) <= 0 )
    {
        fprintf ( stderr, "failed to parse loader size.\n" );
        return -1;
    }

    /* Parse rootfs size */
    if ( hdr_copy_string ( str, sizeof ( str ), header->rootfs_size,
            sizeof ( header->rootfs_size ) ) < 0 || sscanf ( str, "%u", &rootfs_size ) <= 0 )
    {
        fprintf ( stderr, "failed to parse rootfs size.\n" );
        return -1;
    }

    /* Parse kernel size */
    if ( hdr_copy_string ( str, sizeof ( str ), header->kernel_size,
            sizeof ( header->kernel_size ) ) < 0 || sscanf ( str, "%u", &kernel_size ) <= 0 )
    {
        fprintf ( stderr, "failed to parse kernel size.\n" );
        return -1;
    }

    /* Ensure that there are data for checksum calculations */
    if ( length < sizeof ( struct bcm_header_t ) + loader_size + kernel_size + rootfs_size )
    {
        fprintf ( stderr, "no data left to check with crc32.\n" );
        return -1;
    }

    /* Dump header structure fields */
    printf ( "bcm magic      : " );
    print_pad_hex_2 ( ( unsigned char ) header->magic[0] );
    putchar ( '\x20' );
    print_pad_hex_2 ( ( unsigned char ) header->magic[1] );
    putchar ( '\x20' );
    print_pad_hex_2 ( ( unsigned char ) header->magic[2] );
    putchar ( '\x20' );
    print_pad_hex_2 ( ( unsigned char ) header->magic[3] );
    putchar ( '\n' );

    SDUMP ( "bcm vendor     ", header->vendor );
    SDUMP ( "bcm version    ", header->version );
    SDUMP ( "bcm board id   ", header->board_id );
    SDUMP ( "bcm chip id    ", header->chip_id );
    printf ( "cpu endian     : %s ENDIAN\n", header->endian_flag[0] == 0x31 ? "BIG" : "LITTLE" );

    SDUMP2 ( "total  size    ", header->total_size );
    printf ( " (%s)\n", sizeof ( struct bcm_header_t ) + total_size == length ? "ok" : "bad" );
    SDUMP ( "loader addr    ", header->loader_addr );
    SDUMP ( "loader size    ", header->loader_size );
    SDUMP ( "rootfs addr    ", header->rootfs_addr );
    SDUMP ( "rootfs size    ", header->rootfs_size );
    SDUMP ( "kernel addr    ", header->kernel_addr );
    SDUMP ( "kernel size    ", header->kernel_size );

    printf ( "\n------- firmware validation -------\n" );

    /* Get flash size */
    if ( board_ioctl_ar ( boardfd, BOARD_IOCTL_FLASH_READ, FLASH_SIZE, ( int * ) &flash_size ) < 0 )
    {
        return -1;
    }

    /* Validate firmware total length */
    if ( total_size > flash_size )
    {
        fprintf ( stderr, "firmware file bigger than flash memory (%u > %u).\n", total_size,
            flash_size );
        return -1;
    }

    printf ( "size passed.\n" );

    /* Get current board ID */
    if ( board_ioctl_sr ( boardfd, BOARD_IOCTL_GET_ID, board_id, sizeof ( board_id ),
            &result ) < 0 )
    {
        return -1;
    }

    /* Extract board ID string from header */
    if ( hdr_copy_string ( str, sizeof ( str ), header->board_id,
            sizeof ( header->board_id ) ) < 0 )
    {
        return -1;
    }

    /* Validate firmware board ID */
    if ( strcmp ( str, board_id ) )
    {
        fprintf ( stderr, "board ident. mismatched (found: %s, expected: %s)\n", header->board_id,
            board_id );
        return -1;
    }

    printf ( "board id passed.\n" );

    /* Parse firmware chip ID */
    if ( hdr_copy_string ( str, sizeof ( str ), header->chip_id,
            sizeof ( header->chip_id ) ) < 0 || sscanf ( str, "%x", &chip_id ) <= 0 )
    {
        fprintf ( stderr, "failed to parse chip ident.\n" );
        return -1;
    }

    /* Get current chip ID */
    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_CHIP_ID, &result ) < 0 )
    {
        return -1;
    }

    /* Validate firmware chip ID */
    if ( chip_id != result )
    {
        fprintf ( stderr, "chip ident. mismatched (found: %x, expected: %x)\n", chip_id, result );
        return -1;
    }

    printf ( "chip id passed.\n" );

    /* Calculate checksums */
    data_crc32 =
        htonl ( crc32buf ( image + sizeof ( struct bcm_header_t ),
            length - sizeof ( struct bcm_header_t ) ) );
    rootfs_crc32 =
        htonl ( crc32buf ( image + sizeof ( struct bcm_header_t ) + loader_size, rootfs_size ) );
    kernel_crc32 =
        htonl ( crc32buf ( image + sizeof ( struct bcm_header_t ) + loader_size + rootfs_size,
            kernel_size ) );
    header_crc32 = htonl ( crc32buf ( image, 236 ) );

    /* Dump checksums */
    printf ( "\n------- checksum information -------\n" );
    printf ( "data   crc     : 0x" );
    print_pad_hex ( ntohl ( header->data_crc32 ) );
    printf ( " (%s)\n", header->data_crc32 == data_crc32 ? "correct" : "incorrect" );
    printf ( "rootfs crc     : 0x" );
    print_pad_hex ( ntohl ( header->rootfs_crc32 ) );
    printf ( " (%s)\n", header->rootfs_crc32 == rootfs_crc32 ? "correct" : "incorrect" );
    printf ( "kernel crc     : 0x" );
    print_pad_hex ( ntohl ( header->kernel_crc32 ) );
    printf ( " (%s)\n", header->kernel_crc32 == kernel_crc32 ? "correct" : "incorrect" );
    printf ( "sequence       : 0x" );
    print_pad_hex ( header->sequence );
    putchar ( '\n' );
    printf ( "root length    : 0x" );
    print_pad_hex ( header->root_length );
    putchar ( '\n' );
    printf ( "header crc     : 0x" );
    print_pad_hex ( ntohl ( header->header_crc32 ) );
    printf ( " (%s)\n", header->header_crc32 == header_crc32 ? "correct" : "incorrect" );
    putchar ( '\n' );

    /* Stop if at least one checksum is wrong */
    if ( header->data_crc32 != data_crc32
        || header->rootfs_crc32 != rootfs_crc32
        || header->kernel_crc32 != kernel_crc32 || header->header_crc32 != header_crc32 )
    {
        fprintf ( stderr, "header contains bad crc32.\n" );
        return -1;
    }

    return 0;
}

/**
 * Check if bootloader is present in firmware
 */
int firmware_loader_is_present ( const unsigned char *image, unsigned int length, int *result )
{
    unsigned int loader_addr = 0;
    unsigned int loader_size = 0;
    const struct bcm_header_t *header = ( const struct bcm_header_t * ) image;
    char str[64];

    /* Validate firmware size */
    if ( length < sizeof ( struct bcm_header_t ) )
    {
        fprintf ( stderr, "firmware size is invalid.\n" );
        return -1;
    }

    /* Parse firmware header values */
    if ( hdr_copy_string ( str, sizeof ( str ), header->loader_addr,
            sizeof ( header->loader_addr ) ) < 0 || sscanf ( str, "%u", &loader_addr ) <= 0
        || hdr_copy_string ( str, sizeof ( str ), header->loader_size,
            sizeof ( header->loader_size ) ) < 0 || sscanf ( str, "%u", &loader_size ) <= 0 )
    {
        fprintf ( stderr, "failed to parse firmware header.\n" );
        return -1;
    }

    /* Loader is present if its address and size are non-zero values */
    *result = loader_addr && loader_size;

    return 0;
}

/**
 * Flash Broadcom firmware file
 */
int flash_bcm_firmware ( int boardfd, unsigned char *image, unsigned int length )
{
    int result;
    unsigned char *fs_addr = image;
    unsigned int loader_addr = 0;
    unsigned int loader_size = 0;
    unsigned int rootfs_addr = 0;
    unsigned int rootfs_size = 0;
    unsigned int kernel_addr = 0;
    unsigned int kernel_size = 0;
    const struct bcm_header_t *header = ( const struct bcm_header_t * ) image;
    char str[64];

    /* Parse firmware header values */
    if ( hdr_copy_string ( str, sizeof ( str ), header->loader_addr,
            sizeof ( header->loader_addr ) ) < 0 || sscanf ( str, "%u", &loader_addr ) <= 0
        || hdr_copy_string ( str, sizeof ( str ), header->loader_size,
            sizeof ( header->loader_size ) ) < 0 || sscanf ( str, "%u", &loader_size ) <= 0
        || hdr_copy_string ( str, sizeof ( str ), header->rootfs_addr,
            sizeof ( header->rootfs_addr ) ) < 0 || sscanf ( str, "%u", &rootfs_addr ) <= 0
        || hdr_copy_string ( str, sizeof ( str ), header->rootfs_size,
            sizeof ( header->rootfs_size ) ) < 0 || sscanf ( str, "%u", &rootfs_size ) <= 0
        || hdr_copy_string ( str, sizeof ( str ), header->kernel_addr,
            sizeof ( header->kernel_addr ) ) < 0 || sscanf ( str, "%u", &kernel_addr ) <= 0
        || hdr_copy_string ( str, sizeof ( str ), header->kernel_size,
            sizeof ( header->kernel_size ) ) < 0 || sscanf ( str, "%u", &kernel_size ) <= 0 )
    {
        fprintf ( stderr, "failed to parse firmware header.\n" );
        return -1;
    }

    /* Validate firmware size */
    if ( length != sizeof ( struct bcm_header_t ) + loader_size + rootfs_size + kernel_size )
    {
        fprintf ( stderr, "firmware size is corrupted.\n" );
        return -1;
    }

    /* Flash loader is possible */
    if ( loader_addr && loader_size )
    {
        printf ( "flashing CFE...\n" );
        printf ( "layout[loader]: 0x" );
        print_pad_hex ( loader_addr );
        printf ( " +%u\n", loader_size );

        if ( board_ioctl_asor ( boardfd, BOARD_IOCTL_FLASH_WRITE, BCM_IMAGE_CFE,
                ( char * ) ( image + sizeof ( struct bcm_header_t ) ), loader_size,
                ( int ) loader_addr, &result ) < 0 )
        {
            fprintf ( stderr, "failed to flash CFE.\n" );
            return -1;
        }
        printf ( "done flashing CFE.\n" );
    }

    /* Flash rootfs and kernel is possbile */
    if ( rootfs_addr && rootfs_size && kernel_addr && kernel_size )
    {
        /* Show firmware partitions layout */
        printf ( "layout[rootfs]: 0x" );
        print_pad_hex ( rootfs_addr );
        printf ( " +%u\nlayout[kernel]: 0x", rootfs_size );
        print_pad_hex ( kernel_addr );
        printf ( " +%u\n", kernel_size );

        /* Firmware header is always at the first sector of fs */
        if ( loader_addr && loader_size )
        {
            fs_addr += loader_size;
            memcpy ( fs_addr, image, sizeof ( struct bcm_header_t ) );
        }

        /* Start flashing bcm firmware header size before rootfs addr */
        rootfs_addr -= sizeof ( struct bcm_header_t );

        /* Show flashing in process message */
        printf ( "please wait until device reboots...\n" );
        printf ( "flashing rootfs and kernel...\n" );

        if ( board_ioctl_asor ( boardfd, BOARD_IOCTL_FLASH_WRITE, BCM_IMAGE_FS, ( char * ) fs_addr,
                sizeof ( struct bcm_header_t ) + rootfs_size + kernel_size,
                ( int ) ( rootfs_addr ), &result ) < 0 )
        {
            printf ( "failed to flash rootfs and kernel.\n" );
            return -1;
        }

        printf ( "done flashing rootfs and kernel.\n" );
    }

    return 0;
}
