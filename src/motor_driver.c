// #include <ch.h>
#include <hal.h>
#include <math.h>
// #include <usbcfg.h>
// #include <chprintf.h>
#include <motors.h>
#include <sensors/proximity.h>
#include <audio/audio_thread.h>
#include <selector.h>

#include "motor_driver.h"
#include "main.h"
#include "audio_processing.h"

/*PD regulator define*/
#define KP						0.3f
#define KD 						0.0f //1000.0f

/*Speedy rotation define*/
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			1 
#define NO_CORRECTION			0
#define CONST_SPEED             500

/*Sound threshold define*/
#define ERROR_THRESHOLD			1000
#define ACTIVATION_THERSHOLD         2000

/*IR sensors define*/
#define IR_FRONT_LEFT 			7
#define IR_FRONT_RIGHT 			0
#define MIN_DISTANCE_TO_WALL 	110 //equal to approximately 4cm

/*Siren define, for sound emmission*/
#define SIREN_LOOPS             5
#define SIREN_HFREQ             150
#define SIREN_LFREQ             100

/*Selector define for oral presentation only*/
#define SELECTOR_POS_9          8

/**
 * @brief Operating modes of the robot, packed to use one byte only.
 */
typedef enum __attribute__((__packed__)) operating_mode_t 
{
    SILENCE_MODE,    /**< Robot is in silence mode : nothing to do, wait for noise. */
    NOISE_MODE,      /**< Robot is in noise mode : search for the noise source. */
    WALL_DETECTED,   /**< Robot detected a wall : stop and make sound (beacon) with light if there is a wall. */
} operating_mode_t;

/* Static condition for beacon activation */
static bool enabled_giro = false;

/************************* INTERNAL FUNCTIONS **********************************/

/** 
 * @brief Check if there is a wall in front of the robot.
 *
 * @return true if there is a wall, false otherwise.
 */
static bool wall_detection(void){

	if ((get_prox(IR_FRONT_LEFT) > MIN_DISTANCE_TO_WALL) ||
		(get_prox(IR_FRONT_RIGHT) > MIN_DISTANCE_TO_WALL)){
		return true;
	} else {
		return false;
	}
}

/** 
 * @brief Check if a sound is detected by the robot's microphones.
 *
 * @return true if a sound is detected, false otherwise.
 */
static bool sound_detected(void){
	if ((audio_get_diff_intensity_front_left() > ACTIVATION_THERSHOLD) ||
		(audio_get_diff_intensity_front_right() > ACTIVATION_THERSHOLD)){
		return true;
	} else {
		return false;
	}
}

/** 
 * @brief Proportional-Derivative (PD) regulator to correct the robot's trajectory based on the intensity gap between the microphones.
 *
 * @param intensity_gap The difference in intensity between the microphones to correct, shall be 0.
 * @return The speed correction calculated by the PD regulator.
 */
static int16_t pd_regulator(float intensity_gap){	

	float current_error = intensity_gap;
	float past_error = 0;
	float speed = CONST_SPEED;

	past_error = current_error;

	speed = KP * current_error + KD * (current_error - past_error);

    return (int16_t)speed;
}

/** 
 * @brief Set the speed of the robot's motors.
 *
 * @param speed_right Speed of the right motor.
 * @param speed_left Speed of the left motor.
 */
static void set_motor_speed(int16_t speed_right, int16_t speed_left){
	right_motor_set_speed(speed_right);
	left_motor_set_speed(speed_left);
}

/** 
 * @brief Play a siren sound using the robot's speaker.
 */
static void play_siren(void){
	bool siren = 0;
	dac_start();
	for (size_t i = 0; i < SIREN_LOOPS; ++i)
	{
		dac_stop();
		siren =! siren;

		if (siren == 0) {
			dac_start();
			dac_play(SIREN_HFREQ);
			chThdSleepMilliseconds(50);
		} else {
			dac_start();
			dac_play(SIREN_LFREQ);
			chThdSleepMilliseconds(50);
		}
	}
	dac_stop();
}

/**************************** THREAD *************************************/

static THD_WORKING_AREA(waMotorRegulator, 128);
static THD_FUNCTION(MotorRegulator, arg){

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
	
	systime_t time;
    int16_t speed = CONST_SPEED;
    int16_t speed_correction = 0;
	
	// Operating mode variable initialization at SILENCE_MODE
	operating_mode_t mode = SILENCE_MODE;

    while(1){
        time = chVTGetSystemTime();

		switch (mode)
		{
		case SILENCE_MODE:			

			if(sound_detected()){
				enabled_giro = true;
				mode = NOISE_MODE;
			} else {
				enabled_giro = false;
			}
			break;

		case NOISE_MODE:
			enabled_giro = true;

			float diff_intensity = audio_get_diff_intensity_front_right() - audio_get_diff_intensity_front_left();

			if(fabs(diff_intensity) < ERROR_THRESHOLD){
				speed_correction = NO_CORRECTION;
			}else{ 
				speed_correction = pd_regulator(diff_intensity);
			}

			// Following condition is only for the oral presentation, to show the pd controller
			if(get_selector() == SELECTOR_POS_9){			
				speed = NO_CORRECTION;
			} else {
				speed = CONST_SPEED;
			}

			set_motor_speed(speed + ROTATION_COEFF * speed_correction, speed - ROTATION_COEFF * speed_correction);

			// Transition conditions to other modes
			if(wall_detection()){
				mode = WALL_DETECTED;
			}

			if(!sound_detected()){				
				mode = SILENCE_MODE;
			}	
			break;

		case WALL_DETECTED:
		
			set_motor_speed(NO_CORRECTION, NO_CORRECTION);

			if(!wall_detection() && !sound_detected()){
				mode = SILENCE_MODE;
			}
			
			if(!wall_detection() && sound_detected()){
				mode = NOISE_MODE;
			}

			play_siren();
			break;

		default:
			break;
		}
		chThdSleepUntilWindowed(time, time + MS2ST(10));
	}

}

/************************* PUBLIC FUNCTION **********************************/

void motor_regulator_start(void) {
	chThdCreateStatic(waMotorRegulator, sizeof(waMotorRegulator), NORMALPRIO, MotorRegulator, NULL);
}

bool get_enabled_giro(void){
	return enabled_giro;
}

