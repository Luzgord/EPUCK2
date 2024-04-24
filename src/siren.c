#include "ch.h"
#include "hal.h"
#include <usbcfg.h>
#include <chprintf.h>

#include <audio/audio_thread.h>
#include <spi_comm.h> //Leds work even without that
#include <leds.h>
#include "siren.h"

static bool fsiren = 1; //arbitrary value when starting siren

void start_siren(void) {
    chThdCreateStatic(waSiren, sizeof(waSiren), NORMALPRIO, ThdSiren, NULL);
}

static THD_WORKING_AREA(waSiren, 256);
static THD_FUNCTION(ThdSiren, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	set_rgb_led(LED2, RGB_MAX_INTENSITY, 0, 0); //blue leds first
    set_rgb_led(LED4,  RGB_MAX_INTENSITY, 0, 0); //blue leds first
	set_rgb_led(LED8, RGB_MAX_INTENSITY, 0, 0);

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

