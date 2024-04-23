#include <ch.h>
#include <hal.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <arm_math.h>

#include "audio_processing.h"
#include "main.h"
#include "fft.h"
#include "communications.h"

//semaphore
static BSEMAPHORE_DECL(sendToComputer_sem, TRUE);

//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];
static float micFront_cmplx_input[2 * FFT_SIZE];
static float micBack_cmplx_input[2 * FFT_SIZE];
//Arrays containing the computed magnitude of the complex numbers
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];
static float micFront_output[FFT_SIZE];
static float micBack_output[FFT_SIZE];

#define MIN_VALUE_THRESHOLD	10000 

#define MIN_FREQ		10	//we don't analyze before this index to not use resources for nothing
#define FREQ_FORWARD	16	//250Hz
#define FREQ_LEFT		19	//296Hz
#define FREQ_RIGHT		23	//359HZ
#define FREQ_BACKWARD	26	//406Hz
#define MAX_FREQ		30	//we don't analyze after this index to not use resources for nothing

#define FREQ_FORWARD_L		(FREQ_FORWARD-1)
#define FREQ_FORWARD_H		(FREQ_FORWARD+1)
#define FREQ_LEFT_L			(FREQ_LEFT-1)
#define FREQ_LEFT_H			(FREQ_LEFT+1)
#define FREQ_RIGHT_L		(FREQ_RIGHT-1)
#define FREQ_RIGHT_H		(FREQ_RIGHT+1)
#define FREQ_BACKWARD_L		(FREQ_BACKWARD-1)
#define FREQ_BACKWARD_H		(FREQ_BACKWARD+1)

static QUADRANT_NAME_t quadrant_status = QUADRANT_0;
// Taille badasse Ok?
static THD_WORKING_AREA(waAudioProcessingThread, 2048);  
static THD_FUNCTION(AudioProcessingThread, arg) {
	
	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	float *fft_ptr_maxFront_intensity = NULL; // FFT: 0-512 frequences positives croissantes, 513-1023 frequences negatives decroissantes
	float *fft_ptr_maxBack_intensity = NULL;
	float *fft_ptr_maxRight_intensity = NULL;
	float *fft_ptr_maxLeft_intensity = NULL;
	float *fft_ptr_index = NULL; // pointer to the current index of the FFT array
	
	float null_intensity = 0; 
	
	while(1){
		
		// Inits the pointers and search for the max intensity in each microphone data
		//pointer are use to use a minimum of memory
		for(uint16_t i = 0; i < MAX_VALUE_PTR_FFT_SIZE; i++){
			if(i==0){
				fft_ptr_maxFront_intensity = &null_intensity;
				fft_ptr_maxBack_intensity =	 &null_intensity;
				fft_ptr_maxRight_intensity = &null_intensity;
				fft_ptr_maxLeft_intensity = &null_intensity;
			}

			*fft_ptr_index = micFront_output[i];
			if(*fft_ptr_index > *fft_ptr_maxFront_intensity){
				*fft_ptr_maxFront_intensity = *fft_ptr_index;
			}
			
			*fft_ptr_index = micBack_output[i];
			if(*fft_ptr_index > *fft_ptr_maxBack_intensity){
				*fft_ptr_maxBack_intensity = *fft_ptr_index;
			}
			
			*fft_ptr_index = micRight_output[i];
			if(*fft_ptr_index > *fft_ptr_maxRight_intensity){
				*fft_ptr_maxRight_intensity = *fft_ptr_index;
			}
			
			*fft_ptr_index = micLeft_output[i];
			if(*fft_ptr_index > *fft_ptr_maxLeft_intensity){
				*fft_ptr_maxLeft_intensity = *fft_ptr_index;
			}			
		}
// 		     #Front#  
// 		    # Q1|Q2 # 
// 	  Left #---------# Right
// 		    # Q3|Q4 #
//  		 #Back# 
//   		 
		// Search for the quadrant of the max intensity
		if(*fft_ptr_maxFront_intensity - *fft_ptr_maxBack_intensity > 0){ //=> quadrant 1 ou 2 soit aller devant
			if(*fft_ptr_maxRight_intensity - *fft_ptr_maxLeft_intensity < 0){
				quadrant_status = QUADRANT_1;
			}
			else{
				quadrant_status = QUADRANT_2;
			}
		}
		else{ //=> quadrant 3 ou 4 soit aller derriere
			if(*fft_ptr_maxRight_intensity - *fft_ptr_maxLeft_intensity > 0){
				quadrant_status = QUADRANT_4;
			}
			else{
				quadrant_status = QUADRANT_3;
			}
		}
		//send the quadrant to the computer
		send_quadrant_to_computer(quadrant_status);

		}
}


void send_quadrant_to_computer(QUADRANT_NAME_t name){
	chprintf((BaseSequentialStream *)&SDU1, "Quadrant : %d\n", name);
}

/*
*	Simple function used to detect the highest value in a buffer
*	and to execute a motor command depending on it
*/
void sound_remote(float* data){
	float max_norm = MIN_VALUE_THRESHOLD;
	int16_t max_norm_index = -1; 

	//search for the highest peak
	for(uint16_t i = MIN_FREQ ; i <= MAX_FREQ ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}

	//go forward
	if(max_norm_index >= FREQ_FORWARD_L && max_norm_index <= FREQ_FORWARD_H){
		left_motor_set_speed(600);
		right_motor_set_speed(600);
	}
	//turn left
	else if(max_norm_index >= FREQ_LEFT_L && max_norm_index <= FREQ_LEFT_H){
		left_motor_set_speed(-600);
		right_motor_set_speed(600);
	}
	//turn right
	else if(max_norm_index >= FREQ_RIGHT_L && max_norm_index <= FREQ_RIGHT_H){
		left_motor_set_speed(600);
		right_motor_set_speed(-600);
	}
	//go backward
	else if(max_norm_index >= FREQ_BACKWARD_L && max_norm_index <= FREQ_BACKWARD_H){
		left_motor_set_speed(-600);
		right_motor_set_speed(-600);
	}
	else{
		left_motor_set_speed(0);
		right_motor_set_speed(0);
	}
}

/*
*	Callback called when the demodulation of the four microphones is done.
*	We get 160 samples per mic every 10ms (16kHz)
*	
*	params :
*	int16_t *data			Buffer containing 4 times 160 samples. the samples are sorted by micro
*							so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
*	uint16_t num_samples	Tells how many data we get in total (should always be 640)
*/
void processAudioData(int16_t *data, uint16_t num_samples){

	/*
	*
	*	We get 160 samples per mic every 10ms
	*	So we fill the samples buffers to reach
	*	1024 samples, then we compute the FFTs.
	*
	*/

	static uint16_t nb_samples = 0;
	static uint8_t mustSend = 0;

	//loop to fill the buffers
	for(uint16_t i = 0 ; i < num_samples ; i+=4){
		//construct an array of complex numbers. Put 0 to the imaginary part
		micRight_cmplx_input[nb_samples] = (float)data[i + MIC_RIGHT];
		micLeft_cmplx_input[nb_samples] = (float)data[i + MIC_LEFT];
		micBack_cmplx_input[nb_samples] = (float)data[i + MIC_BACK];
		micFront_cmplx_input[nb_samples] = (float)data[i + MIC_FRONT];

		nb_samples++;

		micRight_cmplx_input[nb_samples] = 0;
		micLeft_cmplx_input[nb_samples] = 0;
		micBack_cmplx_input[nb_samples] = 0;
		micFront_cmplx_input[nb_samples] = 0;

		nb_samples++;

		//stop when buffer is full
		if(nb_samples >= (2 * FFT_SIZE)){
			break;
		}
	}

	if(nb_samples >= (2 * FFT_SIZE)){
		/*	FFT proccessing
		*
		*	This FFT function stores the results in the input buffer given.
		*	This is an "In Place" function. 
		*/

		doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
		doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
		doFFT_optimized(FFT_SIZE, micFront_cmplx_input);
		doFFT_optimized(FFT_SIZE, micBack_cmplx_input);

		/*	Magnitude processing
		*
		*	Computes the magnitude of the complex numbers and
		*	stores them in a buffer of FFT_SIZE because it only contains
		*	real numbers.
		*
		*/
		arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
		arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);
		arm_cmplx_mag_f32(micFront_cmplx_input, micFront_output, FFT_SIZE);
		arm_cmplx_mag_f32(micBack_cmplx_input, micBack_output, FFT_SIZE);

		//sends only one FFT result over 10 for 1 mic to not flood the computer
		//sends to UART3
		if(mustSend > 8){
			//signals to send the result to the computer
			chBSemSignal(&sendToComputer_sem);
			mustSend = 0;
		}
		nb_samples = 0;
		mustSend++;

		sound_remote(micLeft_output);
	}
}

void wait_send_to_computer(void){
	chBSemWait(&sendToComputer_sem);
}

float* get_audio_buffer_ptr(BUFFER_NAME_t name){
	if(name == LEFT_CMPLX_INPUT){
		return micLeft_cmplx_input;
	}
	else if (name == RIGHT_CMPLX_INPUT){
		return micRight_cmplx_input;
	}
	else if (name == FRONT_CMPLX_INPUT){
		return micFront_cmplx_input;
	}
	else if (name == BACK_CMPLX_INPUT){
		return micBack_cmplx_input;
	}
	else if (name == LEFT_OUTPUT){
		return micLeft_output;
	}
	else if (name == RIGHT_OUTPUT){
		return micRight_output;
	}
	else if (name == FRONT_OUTPUT){
		return micFront_output;
	}
	else if (name == BACK_OUTPUT){
		return micBack_output;
	}
	else{
		return NULL;
	}
}

void audio_proces_start(void){
    chThdCreateStatic(waAudioProcessingThread, sizeof(waAudioProcessingThread), NORMALPRIO, AudioProcessingThread, NULL);
}
