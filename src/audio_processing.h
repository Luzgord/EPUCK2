#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H


#define FFT_SIZE 				1024	
#define MAX_VALUE_PTR_FFT_SIZE 	512

typedef enum {
	//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
	LEFT_CMPLX_INPUT = 0,
	RIGHT_CMPLX_INPUT,
	FRONT_CMPLX_INPUT,
	BACK_CMPLX_INPUT,
	//Arrays containing the computed magnitude of the complex numbers
	LEFT_OUTPUT,
	RIGHT_OUTPUT,
	FRONT_OUTPUT,
	BACK_OUTPUT
} BUFFER_NAME_t;

typedef enum {									
	QUADRANT_0 = 0,
	QUADRANT_1,						//			Q_0 = test_state					
	QUADRANT_2,						//		Q_1		Q_2
	QUADRANT_3,						//		Q_3		Q_4
	QUADRANT_4
} QUADRANT_NAME_t;

void audio_proces_start(void);

void processAudioData(int16_t *data, uint16_t num_samples);

/*
*	put the invoking thread into sleep until it can process the audio datas
*/
void wait_send_to_computer(void);

/*
*	Returns the pointer to the BUFFER_NAME_t buffer asked
*/
float* get_audio_buffer_ptr(BUFFER_NAME_t name);

// void reset_all_pointer_values(float *Front,float *,float *,float *)

void send_quadrant_to_computer(QUADRANT_NAME_t name);

#endif /* AUDIO_PROCESSING_H */