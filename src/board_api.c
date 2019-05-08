/* ------------------------------------------------------------------
 * Broadcom Board API Support
 * ------------------------------------------------------------------ */

#include "brfw.h"

/**
 * Perform broard ioctl operation
 */
int board_ioctl_asor ( int fd, int request, int action, char *string, int slen, int offset,
    int *result )
{
    struct board_ioctl_params_t ctrlParms = {
        string, NULL, slen, offset, action, 0
    };

    if ( ( unsigned int ) request != BOARD_IOCTL_FLASH_WRITE && string && slen )
    {
        memset ( string + 1, '\0', slen - 1 );
        string[0] = '?';
    }

    if ( ioctl ( fd, request, &ctrlParms ) < 0 )
    {
        fprintf ( stderr, "ioctl failed (r=0x%x a=0x%x s=0x%lx l=0x%x o=0x%x): %i\n", request,
            action, ( unsigned long ) string, slen, offset, errno );
        return -1;
    }

    if ( result )
    {
        *result = ctrlParms.result;
    }

    return 0;
}

/**
 * Perform broard ioctl operation
 */
int board_ioctl ( int fd, int request )
{
    return board_ioctl_asor ( fd, request, 0, NULL, 0, 0, NULL );
}

/**
 * Perform broard ioctl operation (action, result)
 */
int board_ioctl_ar ( int fd, int request, int action, int *result )
{
    return board_ioctl_asor ( fd, request, action, NULL, 0, 0, result );
}

/**
 * Perform broard ioctl operation (result)
 */
int board_ioctl_r ( int fd, int request, int *result )
{
    return board_ioctl_asor ( fd, request, 0, NULL, 0, 0, result );
}

/**
 * Perform broard ioctl operation (string, result)
 */
int board_ioctl_sr ( int fd, int request, char *string, size_t slen, int *result )
{
    return board_ioctl_asor ( fd, request, 0, string, slen, 0, result );
}

/**
 * Perform broard ioctl operation (string, offset, result)
 */
int board_ioctl_sor ( int fd, int request, char *string, int slen, int offset, int *result )
{
    return board_ioctl_asor ( fd, request, 0, string, slen, offset, result );
}
