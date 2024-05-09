/**
 * @file     siren.c
 * @brief    This file contains functions for controlling the siren.
**/

/*File from e-puck library*/
#include <ch.h>
#include <hal.h>
#include <usbcfg.h>
#include <leds.h>
/*Specific files*/
#include "siren.h"
#include "motor_driver.h"

/**************************** THREAD *************************************/

/* Siren thread, used to make the robot blink with its LEDs when a sound is detected.
   The robot will blink with in a X pattern as shown below:
    RGB_RED      ####   RGB_BLUE
           \   ##    ##  /
             #   FRONT   # 
           #              #     
           #              #
             #   BACK   #
          /    ##    ##   \
    RGB_BLUE     ####     RGB_RED
*/

static THD_WORKING_AREA(waSiren, 16);
static THD_FUNCTION(ThdSiren, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
	/*Condition use to have a X blink pattern*/
	bool first_blink = true;

    while(1) {
		if(get_enabled_giro()){
			if(first_blink){
				set_rgb_led(LED2, 0, 0, RGB_MAX_INTENSITY); 
				set_rgb_led(LED6,  0, 0, RGB_MAX_INTENSITY);
				first_blink = false;
			} else {
				toggle_rgb_led(LED2, BLUE_LED, RGB_MAX_INTENSITY);
				toggle_rgb_led(LED4, RED_LED, RGB_MAX_INTENSITY);
				toggle_rgb_led(LED6, BLUE_LED, RGB_MAX_INTENSITY);
				toggle_rgb_led(LED8, GREEN_LED, RGB_MAX_INTENSITY);
			}
			
		} else {
			clear_leds();
			first_blink = true;
		}
		chThdSleepMilliseconds(500);
	}
}

/************************* PUBLIC FUNCTION **********************************/

void siren_start(void) {
    chThdCreateStatic(waSiren, sizeof(waSiren), NORMALPRIO, ThdSiren, NULL);
}
