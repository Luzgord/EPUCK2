#include "ch.h"
#include "hal.h"
#include <usbcfg.h>
#include <chprintf.h>

#include <audio/audio_thread.h>
#include <spi_comm.h> //Leds work even without that
#include <leds.h>
#include "siren.h"

static bool fsiren = true; //arbitrary value when starting siren


static THD_WORKING_AREA(waSiren, 256);
static THD_FUNCTION(ThdSiren, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	set_rgb_led(LED2, 0, 0, RGB_MAX_INTENSITY); //blue leds first
    //set_rgb_led(LED4,  0, 0, RGB_MAX_INTENSITY); //blue leds first
	set_rgb_led(LED6,  0, 0, RGB_MAX_INTENSITY); //blue leds first
	//set_rgb_led(LED8, 0, 0, RGB_MAX_INTENSITY);
	dac_start(); //stop first frequency then play next one

    while(1) {
        fsiren =! fsiren; //invert value

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

void start_siren(void) {
    chThdCreateStatic(waSiren, sizeof(waSiren), NORMALPRIO, ThdSiren, NULL);
}
