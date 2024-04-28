#include <ch.h>
#include <hal.h>
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>

#include "pi_regulator.h"
#include "main.h"
#include "audio_processing.h"

static bool enabled_motors = false;

extern float diff_intesity_avg_left_right;

//simple PI regulator implementation
int16_t pi_regulator(float ecart_intensite){ //we want ecart_intensite to be 0 => goal = 0

	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = ecart_intensite;

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the mic are a bit noisy

	if(fabs(error) < ERROR_THRESHOLD){
		return 0;
	}

	sum_error += error;

	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth

	if(sum_error > MAX_SUM_ERROR){
		sum_error = MAX_SUM_ERROR;
	}else if(sum_error < -MAX_SUM_ERROR){
		sum_error = -MAX_SUM_ERROR;
	}

	speed = KP * error + KI * sum_error;

    return (int16_t)speed;
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 100;
    int16_t speed_correction = 0;

    while(1){
        time = chVTGetSystemTime();
        
        //100 is the speed to go straight
        // speed = 100; 
        //computes a correction factor to let the robot rotate to be in front of the noise
		// float diff_intensity = audio_get_diff_intensity();
        speed_correction = pi_regulator(diff_intesity_avg_left_right);

        //if the line is nearly in front of the camera, don't rotate
        if(abs(speed_correction) < ROTATION_THRESHOLD){
        	speed_correction = 0;
        }

		if (enabled_motors){
			//applies the speed from the PI regulator and the correction for the rotation
			right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
			left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
		}
		else {
			//stop the motors
			right_motor_set_speed(0);
			left_motor_set_speed(0);
		}

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void) {
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

void set_enabled_motors(bool enable) {
	enabled_motors = enable;
}

void toogle_enabled_motors() {
	enabled_motors = !enabled_motors;
}