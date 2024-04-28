#include "ch.h"
#include "hal.h"
#include "siren.h"
#include <usbcfg.h>
#include <chprintf.h>
#include <audio/audio_thread.h>
#include <leds.h>

static bool fsiren = 1;


static THD_WORKING_AREA(waSiren, 256);
static THD_FUNCTION(ThdSiren, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	set_rgb_led(LED2, 0, 0, RGB_MAX_INTENSITY); //blue leds first
	set_rgb_led(LED6,  0, 0, RGB_MAX_INTENSITY);
	dac_start(); //activating the dac

    while(1) {
        fsiren =! fsiren; //to change pitch of the siren

		//toggling the LEDS in an X pattern

		toggle_rgb_led(LED2, BLUE_LED, RGB_MAX_INTENSITY);
		toggle_rgb_led(LED4, RED_LED, RGB_MAX_INTENSITY);
		toggle_rgb_led(LED6, BLUE_LED, RGB_MAX_INTENSITY);
		toggle_rgb_led(LED8, RED_LED, RGB_MAX_INTENSITY);

		// if (fsiren == false) {
		// 	dac_stop();
		// 	dac_play(100);
		// } else {
		// 	dac_stop();
		// 	dac_play(150);
		// }

		chThdSleepMilliseconds(500);
	}
}

void siren_start(void) {
    chThdCreateStatic(waSiren, sizeof(waSiren), NORMALPRIO, ThdSiren, NULL);
}
