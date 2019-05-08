/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Flash Firmware Support
 * ------------------------------------------------------------------ */

#include "brfw.h"

/**
 * Prompt firmwmare upgrade process
 */
static int prompt_operation ( const char *desc )
{
    char ans[2];

    printf ( "%s [y/N] ", desc );

    if ( read ( 0, ans, sizeof ( ans ) ) != 2 )
    {
        return 0;
    }

    return ans[0] == 'y' && ans[1] == '\n';
}

/**
 * Flash firmware file (CFE, kernel and rootfs)
 */
int flash_firmware ( int boardfd, const char *path, int flags )
{
    int fd;
    int loader_is_present;
    unsigned int size;
    unsigned int length;
    unsigned char *image;

    /* Open firmware file for reading */
    if ( ( fd = open ( path, O_RDONLY ) ) < 0 )
    {
        perror ( path );
        return -1;
    }

    /* Measure firmware file length */
    if ( ( length = lseek ( fd, 0, SEEK_END ) ) == ( unsigned int ) -1 )
    {
        perror ( "lseek" );
        close ( fd );
        return -1;
    }

    /* Restore firmware file position */
    if ( lseek ( fd, 0, SEEK_SET ) == ( off_t ) - 1 )
    {
        perror ( "lseek" );
        close ( fd );
        return -1;
    }

    /* Calculate pages needed for memory mapping */
    size = ( ( length + PAGESIZE - 1 ) / PAGESIZE ) * PAGESIZE;

    /* Allocate buffer for firmware data */
    if ( ( image =
            ( unsigned char * ) ( struct memblk_t * ) mmap ( NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) ) == MAP_FAILED )
    {
        perror ( "mmap" );
        close ( fd );
        return -1;
    }

    /* Read firmware data into buffer */
    if ( read ( fd, image, length ) != ( int ) length )
    {
        perror ( "read" );
        return -1;
    }

    /* Close firmware file fd */
    close ( fd );

    /* Verify firmware file */
    if ( verify_bcm_firmware ( boardfd, image, length ) < 0 )
    {
        printf ( "firmware file is broken.\n" );
        munmap ( image, size );
        return -1;
    }

    /* Check if bootloader is present in firmware */
    if ( firmware_loader_is_present ( image, length, &loader_is_present ) < 0 )
    {
        munmap ( image, size );
        return -1;
    }

    /* Confirm firmware upgrade if needed */
    if ( ~flags & FLASH_NO_PROMPT && loader_is_present )
    {
        if ( !prompt_operation ( "CFE bootloader will be flashed, is that ok?" ) )
        {
            fprintf ( stderr, "firmware upgrade canceled.\n" );
            munmap ( image, size );
            return -1;
        }
    }

    /* Confirm firmware upgrade if needed */
    if ( ~flags & FLASH_NO_PROMPT )
    {
        if ( !prompt_operation ( "Are you sure to flash firmware?" ) )
        {
            fprintf ( stderr, "firmware upgrade canceled.\n" );
            munmap ( image, size );
            return -1;
        }
    }

    /* Flash firmware file */
    if ( flash_bcm_firmware ( boardfd, image, length ) < 0 )
    {
        munmap ( image, size );
        return -1;
    }

    /* Free memory map */
    munmap ( image, size );

    return 0;
}
