#include <ch.h>
#include <hal.h>
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>
#include <sensors/proximity.h>
#include <audio/audio_thread.h>

#include "motor_driver.h"
#include "main.h"
#include "audio_processing.h"

static bool enabled_motors = true;

bool wall_detection(void){

	if ((get_prox(IR_FRONT_LEFT) > MIN_DISTANCE_TO_WALL) ||
		(get_prox(IR_FRONT_RIGHT) > MIN_DISTANCE_TO_WALL)){
		return true;
	} else {
		return false;
	}
}

//Regulator implementation
int16_t p_regulator(float intensity_gap){	//we want intensity_gap to be 0

	float current_error = intensity_gap;
	float past_error = 0;
	static float sum_error = 0;
	float speed = 0;

	past_error = current_error;
	sum_error += current_error;

	//we set max and a min for the sum to avoid an uncontrolled growth
	if(sum_error > MAX_SUM_ERROR){
		sum_error = MAX_SUM_ERROR;
	}else if(sum_error < -MAX_SUM_ERROR){
		sum_error = -MAX_SUM_ERROR;
	}

	speed = KP * current_error + KI * sum_error + KD * (current_error - past_error);

    return (int16_t)speed;
}

static THD_WORKING_AREA(waMotorRegulator, 256);
static THD_FUNCTION(MotorRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
	systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;

    while(1){
        time = chVTGetSystemTime();
		
		float diff_intensity = audio_get_diff_intensity_front_right() -
							   audio_get_diff_intensity_front_left();	
		
		if(wall_detection()){		//sound sirens if wall detected
			bool siren = 0;
			for (size_t i = 0; i < SIREN_LOOPS; ++i)
			{
				dac_start();
				bool siren =! siren;

				if (siren == 0) {
					dac_stop();
					dac_play(SIREN_HFREQ);
					chThdSleepMilliseconds(50);
				} else {
					dac_stop();
					dac_play(SIREN_LFREQ);
					chThdSleepMilliseconds(50);
				}
			}
		}
		//when siren drill is over, resume normal operation

		//high pass filter, avoid too low values of intensity difference
		if(fabs(diff_intensity) < ERROR_THRESHOLD){
			speed_correction = NO_CORRECTION;
		}else{ 
			speed_correction = p_regulator(diff_intensity);
		}

        //if the sound is nearly in front of the camera, don't rotate
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
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void motor_regulator_start(void) {
	chThdCreateStatic(waMotorRegulator, sizeof(waMotorRegulator), NORMALPRIO, MotorRegulator, NULL);
}