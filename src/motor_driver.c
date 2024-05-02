#include <ch.h>
#include <hal.h>
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>
#include <sensors/proximity.h>

#include "motor_driver.h"
#include "main.h"
#include "audio_processing.h"

static bool enabled_motors = true;
static bool wall_detected = false; 

void wall_detection(void){
	int IR_L = get_prox(IR_FRONT_LEFT);
	int IR_R = get_prox(IR_FRONT_RIGHT);
	// print_IR_values(IR_L, IR_R);

	if ((get_prox(IR_FRONT_LEFT) > MIN_DISTANCE_TO_WALL) || (get_prox(IR_FRONT_RIGHT) > MIN_DISTANCE_TO_WALL)){
		wall_detected = true;
	} else {
		wall_detected = false;
	}
}

//simple P regulator implementation
int16_t p_regulator(float ecart_intensite){ //we want ecart_intensite to be 0 => goal = 0

	float current_error = ecart_intensite;
	float speed = 100;

	speed = KP * current_error;

    return (int16_t)speed;
}

static THD_WORKING_AREA(waMotorRegulator, 1024); // sans printf  remettre à 256 
static THD_FUNCTION(MotorRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
	systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;

    while(1){
        time = chVTGetSystemTime();
		
		float diff_intensity = audio_get_diff_intensity_front_right() - audio_get_diff_intensity_front_left();	
		
		wall_detection();

		// High pass filter, avoid too low values of intensity difference
		if(fabs(diff_intensity) < ERROR_THRESHOLD){
			speed_correction = NO_CORRECTION;
		}else{ 
			speed_correction = p_regulator(diff_intensity);
		}

        // If the sound is nearly in front of the camera, don't rotate
        if(abs(speed_correction) < ROTATION_THRESHOLD){
        	speed_correction = NO_CORRECTION;
        }

		if (enabled_motors){
			//applies the speed from the PI regulator and the correction for the rotation
			right_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
			left_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
		}
		else {
			//stop the motors
			right_motor_set_speed(0);
			left_motor_set_speed(0);
		}

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10)); // 5ms OK ? avanr c'était 10 => 100Hz mais trop lent 
    }
}

// void print_IR_values(int IR_L, int IR_R){
// 	chprintf((BaseSequentialStream *)&SDU1, "IR_L: %d, IR_R: %d\n", IR_L, IR_R);
// }

void motor_regulator_start(void) {
	chThdCreateStatic(waMotorRegulator, sizeof(waMotorRegulator), NORMALPRIO, MotorRegulator, NULL);
}

void set_enabled_motors(bool enable) {
	enabled_motors = enable;
}

void toogle_enabled_motors() {
	enabled_motors = !enabled_motors;
}