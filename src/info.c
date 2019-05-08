/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Board Information Print
 * ------------------------------------------------------------------ */

#include "brfw.h"

/**
 * Format and print MAC address
 */
static void print_mac ( const void *mac )
{
    size_t i;
    for ( i = 0; i < 6; i++ )
    {
        if ( ( ( const unsigned char * ) mac )[i] < 16 )
        {
            putchar ( '0' );
        }
        printf ( "%x", ( ( const unsigned char * ) mac )[i] );
        putchar ( i < 5 ? ':' : '\n' );
    }
}

/**
 * Print board details
 */
void print_board_info ( int boardfd )
{
    int result;
    int i;
    int n_enet_macs = 0;
    unsigned short val_ushort = 0;
    unsigned long val_ulong = 0;
    char string[256];

    printf ( "\n------- board information -------\n" );

    if ( board_ioctl_ar ( boardfd, BOARD_IOCTL_FLASH_READ, FLASH_SIZE, &result ) >= 0 )
    {
        printf ( "flash size     : %i\n", result );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_PSI_SIZE, &result ) >= 0 )
    {
        printf ( "psi size       : %i\n", result );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_SDRAM_SIZE, &result ) >= 0 )
    {
        printf ( "sdram size     : %i\n", result );
    }

    if ( board_ioctl_sr ( boardfd, BOARD_IOCTL_GET_ID, string, sizeof ( string ), &result ) >= 0 )
    {
        printf ( "board ident.   : %s\n", string );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_CHIP_ID, &result ) >= 0 )
    {
        printf ( "chip ident.    : %x\n", result );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_CHIP_REV, &result ) >= 0 )
    {
        printf ( "chip revision  : %x\n", result );
    }

    if ( board_ioctl_sr ( boardfd, BOARD_IOCTL_GET_CFE_VER, string, sizeof ( string ),
            &result ) >= 0 )
    {
        printf ( "cfe version    : %u.%u.%u-%u.%u\n",
            ( unsigned char ) string[0],
            ( unsigned char ) string[1],
            ( unsigned char ) string[2], ( unsigned char ) string[3], ( unsigned char ) string[4] );
    }

    if ( board_ioctl_sr ( boardfd, BOARD_IOCTL_GET_BASE_MAC_ADDRESS, string, sizeof ( string ),
            &result ) >= 0 )
    {
        printf ( "base mac       : " );
        print_mac ( string );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_NUM_ENET_MACS, &n_enet_macs ) >= 0 )
    {
        printf ( "num enet macs  : %i\n", n_enet_macs );
    }

    for ( i = 0; i < n_enet_macs; i++ )
    {
        if ( board_ioctl_sor ( boardfd, BOARD_IOCTL_GET_MAC_ADDRESS, string, sizeof ( string ), i,
                &result ) >= 0 )
        {
            printf ( "enet mac [%i]   : ", i );
            print_mac ( string );
        }
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_NUM_ENET_PORTS, &result ) >= 0 )
    {
        printf ( "num enet ports : %i\n", result );
    }

    if ( board_ioctl_sr ( boardfd, BOARD_IOCTL_GET_TRIGGER_EVENT, ( char * ) val_ulong,
            sizeof ( val_ulong ), &result ) >= 0 )
    {
        printf ( "trigger event  : %lx\n", val_ulong );
    }

    if ( board_ioctl_sr ( boardfd, BOARD_IOCTL_GET_WLAN_ANT_INUSE, ( char * ) &val_ushort,
            sizeof ( val_ushort ), &result ) >= 0 )
    {
        printf ( "wlan ant inuse : %x\n", val_ushort );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_BACKUP_PSI_SIZE, &result ) >= 0 )
    {
        printf ( "psi backup     : %i\n", result );
    }

    if ( board_ioctl_r ( boardfd, BOARD_IOCTL_GET_SYSLOG_SIZE, &result ) >= 0 )
    {
        printf ( "syslog size    : %i\n", result );
    }

    if ( ( result = board_ioctl ( boardfd, BOARD_IOCTL_GET_TIMEMS ) ) >= 0 )
    {
        printf ( "time ms        : %i\n", result );
    }
}
