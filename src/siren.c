#include <ch.h>
#include <hal.h>
#include <leds.h>
#include <audio/audio_thread.h>
#include "siren.h"

static bool fsiren = 1; //arbitrary value when starting siren

static THD_WORKING_AREA(waSiren, 128);
static THD_FUNCTION(ThdSiren, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    set_rgb_led(LED4, 0, 0, RGB_MAX_INTENSITY); //blue leds first
	set_rgb_led(LED8, 0, 0, RGB_MAX_INTENSITY);

    while(1) {
        fsiren =! fsiren; //invert value
		dac_stop();
		dac_start(); //stop first frequency then play next one

		toggle_rgb_led(LED2, BLUE_LED, RGB_MAX_INTENSITY);
		toggle_rgb_led(LED4, RED_LED, RGB_MAX_INTENSITY);
		toggle_rgb_led(LED6, BLUE_LED, RGB_MAX_INTENSITY);
		toggle_rgb_led(LED8, RED_LED, RGB_MAX_INTENSITY);

        if(fsiren == false) { //how to reduce intensity of sound?
			dac_play(600);
		} else {
			dac_play(900);
		}

		chThdSleepMilliseconds(500);
	}
}

void siren_start(void) {
    chThdCreateStatic(waSiren, sizeof(waSiren), NORMALPRIO, ThdSiren, NULL);
}