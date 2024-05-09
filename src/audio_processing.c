/* File use from the e-puck library**/
#include <ch.h>
#include <hal.h>
#include <usbcfg.h>
// #include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
// #include <arm_math.h>

/* Specific files for this project**/
#include "audio_processing.h"
#include "main.h"
#include "fft.h"
// #include "communications.h"

/* Minimum value to detect a peak**/
#define FFT_SIZE 				1024	
#define MAX_VALUE_PTR_FFT_SIZE 	512
/* Frequency range for analysis**/
#define MIN_FREQ            10	        // We don't analyze before this index equivalent to approximately 156[Hz] 
#define MAX_FREQ            30	        // We don't analyze after this index equivalent to approximately 469[Hz] 

/* Two times FFT_SIZE because these arrays contain complex numbers (real + imaginary) */
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];
static float micFront_cmplx_input[2 * FFT_SIZE];
static float micBack_cmplx_input[2 * FFT_SIZE];

/* Arrays containing the computed magnitude of the complex numbers */
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];
static float micFront_output[FFT_SIZE];
static float micBack_output[FFT_SIZE];

/* Values used to compute the orientation */
static float diff_intensity_avg_front_left = 0;
static float diff_intensity_avg_front_right = 0;

/************************* INTERNAL FUNCTIONS **********************************/

/**
 * @brief Calculates the average intensity of a buffer.
 *
 * @param buffer Buffer to calculate the average intensity from.
 * @param average_value Pointer to store the average intensity value.
**/
static void calculate_average_intensity(float *buffer, float *average_value)
{
    for (uint16_t i = MIN_FREQ; i <= MAX_FREQ; i++) // Band pass filter
    {   
        *average_value += buffer[i];
    }
    *average_value = *average_value / (float)(MAX_FREQ - MIN_FREQ);
}

/**
 * @brief Function to find the direction of the sound.
**/
void find_direction(void)
{
    float avg_front_intensity = 0;
    float avg_right_intensity = 0;
    float avg_left_intensity = 0;

    float *ptr_avg_front_intensity = &avg_front_intensity;
    float *ptr_avg_right_intensity = &avg_right_intensity;
    float *ptr_avg_left_intensity = &avg_left_intensity;

    calculate_average_intensity(micLeft_output, ptr_avg_left_intensity);
    calculate_average_intensity(micRight_output, ptr_avg_right_intensity);
    calculate_average_intensity(micFront_output, ptr_avg_front_intensity);

    diff_intensity_avg_front_left = *ptr_avg_front_intensity - *ptr_avg_left_intensity;
    diff_intensity_avg_front_right = *ptr_avg_front_intensity - *ptr_avg_right_intensity;
}


/************************* EXTERNAL FUNCTION **********************************/

/**
 * @brief Callback called when the demodulation of the four microphones is done.
 * We get 160 samples per mic every 10ms (16kHz)
 *
 * @param data Buffer containing 4 times 160 samples. The samples are sorted by mic
 * so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
 * @param num_samples Tells how many data we get in total (should always be 640)
**/
void processAudioData(int16_t *data, uint16_t num_samples)
{
    static uint16_t nb_samples = 0;

    // Loop to fill the buffers
    for (uint16_t i = 0; i < num_samples; i += 4)
    {
        // Construct an array of complex numbers. Put 0 to the imaginary part
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

        // Stop when buffer is full
        if (nb_samples >= (2 * FFT_SIZE))
        {
            break;
        }
    }

    if (nb_samples >= (2 * FFT_SIZE))
    {
        /* FFT processing**/
        doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
        doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
        doFFT_optimized(FFT_SIZE, micFront_cmplx_input);
        doFFT_optimized(FFT_SIZE, micBack_cmplx_input);

        /* Magnitude processing**/
        arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
        arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);
        arm_cmplx_mag_f32(micFront_cmplx_input, micFront_output, FFT_SIZE);
        arm_cmplx_mag_f32(micBack_cmplx_input, micBack_output, FFT_SIZE);

        nb_samples = 0;

        // To find the direction of the sound
        find_direction();
    }
}

// /**
//  * @brief Returns a pointer to a specified audio buffer.
//  *
//  * @param name Name of the buffer to get.
//  * @return Pointer to the requested audio buffer.
// **/
// float *get_audio_buffer_ptr(BUFFER_NAME_t name)
// {
//     if (name == LEFT_CMPLX_INPUT)
//     {
//         return micLeft_cmplx_input;
//     }
//     else if (name == RIGHT_CMPLX_INPUT)
//     {
//         return micRight_cmplx_input;
//     }
//     else if (name == FRONT_CMPLX_INPUT)
//     {
//         return micFront_cmplx_input;
//     }
//     else if (name == BACK_CMPLX_INPUT)
//     {
//         return micBack_cmplx_input;
//     }
//     else if (name == LEFT_OUTPUT)
//     {
//         return micLeft_output;
//     }
//     else if (name == RIGHT_OUTPUT)
//     {
//         return micRight_output;
//     }
//     else if (name == FRONT_OUTPUT)
//     {
//         return micFront_output;
//     }
//     else if (name == BACK_OUTPUT)
//     {
//         return micBack_output;
//     }
//     else
//     {
//         return NULL;
//     }
// }

/**
 * @brief Getter for the average difference in intensity for the front left audio.
 * 
 * @return The average difference in intensity for the front left audio.
 */
float audio_get_diff_intensity_front_left(void)
{
    return diff_intensity_avg_front_left;
}

/**
 * @brief Getter for the average difference in intensity for the front right audio.
 * 
 * @return The average difference in intensity for the front right audio.
 */
float audio_get_diff_intensity_front_right(void)
{
    return diff_intensity_avg_front_right;
}
