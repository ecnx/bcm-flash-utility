/* ------------------------------------------------------------------
 * Broadcom Flash Writer - Led Control Support
 * ------------------------------------------------------------------ */

#include "brfw.h"

/**
 * Turn on or off single led
 */
int set_led_state ( int boardfd, unsigned int num, int val )
{
    return board_ioctl_sor ( boardfd, BOARD_IOCTL_LED_CTRL, NULL, ( int ) num, val,
        NULL ) < 0 ? -1 : 0;
}

/**
 * Set state of all available leds
 */
static void blink_leds_in ( int boardfd, int state )
{
    unsigned int led;

    for ( led = 0; led < 16; led++ )
    {
        set_led_state ( boardfd, led, state );
    }
}

/**
 * Blink all leds found on the board n times
 */
void blink_leds ( int boardfd, unsigned int count )
{
    unsigned int i;

    for ( i = 0; i < count; i++ )
    {
        blink_leds_in ( boardfd, 1 );
        usleep ( 200000 );
        blink_leds_in ( boardfd, 0 );
        usleep ( 200000 );
        printf ( "[brfw] leds blinked #%i/%i\n", i + 1, count );
    }
}
