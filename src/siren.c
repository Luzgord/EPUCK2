#include <.git>
#include <hal.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <leds.h>

#include "siren.h"
#include "motor_driver.h"

static THD_WORKING_AREA(waSiren, 256);
static THD_FUNCTION(ThdSiren, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	set_rgb_led(LED2, 0, 0, RGB_MAX_INTENSITY); //blue LEDS first
	set_rgb_led(LED6,  0, 0, RGB_MAX_INTENSITY);

    while(1) {
		//sirens always active
		//toggling the LEDS in an X pattern
		if(get_enabled_giro()){
			toggle_rgb_led(LED2, BLUE_LED, RGB_MAX_INTENSITY);
			toggle_rgb_led(LED4, RED_LED, RGB_MAX_INTENSITY);
			toggle_rgb_led(LED6, BLUE_LED, RGB_MAX_INTENSITY);
			toggle_rgb_led(LED8, RED_LED, RGB_MAX_INTENSITY);
		} else {
			clear_leds();
		}

		chThdSleepMilliseconds(500);
	}
}

void siren_start(void) {
    chThdCreateStatic(waSiren, sizeof(waSiren), NORMALPRIO, ThdSiren, NULL);
}
