/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Main Source File
 * ------------------------------------------------------------------ */

#include "brfw.h"

/**
 * Show program usage message
 */
static void show_usage ( void )
{
    fprintf ( stderr, "usage: brfw option args...\n"
        "\n"
        "options:\n"
        "  -i           show board information\n"
        "  -l num val   set led state to value\n"
        "  -b [count]   blink all the leds found\n"
        "  -r           restart system now\n"
        "  -f file      flash firmware file\n"
        "  -n file      flash file no prompt\n" "  -h           show help message\n" "\n" );
}

/**
 * Perform mips soft reset of system
 */
int mips_soft_reset ( int boardfd )
{
    printf ( "restarting system now...\n" );
    if ( board_ioctl ( boardfd, BOARD_IOCTL_MIPS_SOFT_RESET ) < 0 )
    {
        return -1;
    }

    return 0;
}

/**
 * Program entry point
 */
int main ( int argc, char *argv[] )
{
    int status = 0;
    int boardfd;
    unsigned int num = 1;
    int val;

    /* Show program version */
    printf ( "Broadcom Flash Writer - ver. " BRFW_VERSION "\n" );

    /* Ensure that header size is correct */
    if ( sizeof ( struct bcm_header_t ) != 256 )
    {
        return EINVAL;
    }

    /* Validate arguments count and options count */
    if ( argc < 2 || argv[1][0] != '-' || strlen ( argv[1] ) != 2 )
    {
        show_usage (  );
        return 1;
    }

    /* Open board device */
    if ( ( boardfd = open ( "/dev/brcmboard", O_RDWR ) ) < 0 )
    {
        perror ( "open" );
        return 1;
    }

    /* Perform requested action */
    switch ( argv[1][1] )
    {
    case 'i':
        print_board_info ( boardfd );
        break;
    case 'l':
        if ( argc < 4 || sscanf ( argv[2], "%u", &num ) <= 0
            || sscanf ( argv[3], "%i", &val ) <= 0 )
        {
            show_usage (  );
            status = -1;
            break;
        }
        if ( ( status = set_led_state ( boardfd, num, val ) ) >= 0 )
        {
            printf ( "led #%i set to: %i\n", num, val );
        }
        break;
    case 'b':
        if ( argc > 2 && sscanf ( argv[2], "%u", &num ) <= 0 )
        {
            show_usage (  );
            status = -1;
            break;
        }
        blink_leds ( boardfd, num );
        break;
    case 'r':
        status = mips_soft_reset ( boardfd );
        break;
    case 'f':
        if ( argc < 3 )
        {
            show_usage (  );
            status = -1;
            break;
        }
        status = flash_firmware ( boardfd, argv[2], 0 );
        break;
    case 'n':
        if ( argc < 3 )
        {
            show_usage (  );
            status = -1;
            break;
        }
        status = flash_firmware ( boardfd, argv[2], FLASH_NO_PROMPT );
        break;
    default:
        show_usage (  );
        status = -1;
    }

    /* Close board device fd */
    close ( boardfd );

    if ( status )
    {
        printf ( "operation failed.\n" );
    } else
    {
        printf ( "operation success.\n" );
    }

    return status ? 1 : 0;
}
