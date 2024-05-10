/**
 * @file     lights.c
 * @brief    This file contains functions for controlling the lights.
**/

/*File from e-puck library*/
#include <ch.h>
#include <hal.h>
#include <usbcfg.h>
#include <leds.h>
/*Specific files*/
#include "lights.h"
#include "motor_driver.h"

/**************************** THREAD *************************************/

/* Lights thread, used to make the robot blink with its LEDs when a sound is detected.
   The robot will blink with in a X pattern as shown below:
    RGB_RED      ####   RGB_BLUE
           \  ##      ##  /
            #    FRONT   # 
           #              #     
           #              #
            #    BACK    #
          /   ##      ##   \
    RGB_BLUE     ####     RGB_RED
*/

static THD_WORKING_AREA(waLights, 16);
static THD_FUNCTION(ThdLights, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
	//Condition use to have a X blink pattern
	bool first_blink = true;

    while(1) {
		if(get_enabled_lights()){
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

/************************* PUBLIC FUNCTIONS **********************************/

void lights_start(void) {
    chThdCreateStatic(waLights, sizeof(waLights), NORMALPRIO, ThdLights, NULL);
}
