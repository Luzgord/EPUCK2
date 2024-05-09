#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stdint.h>

typedef enum {
    // 2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
    LEFT_CMPLX_INPUT = 0,
    RIGHT_CMPLX_INPUT,
    FRONT_CMPLX_INPUT,
    BACK_CMPLX_INPUT,
    // Arrays containing the computed magnitude of the complex numbers
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    FRONT_OUTPUT,
    BACK_OUTPUT
} BUFFER_NAME_t;

/**
 * @brief Callback called when the demodulation of the four microphones is done.
 * We get 160 samples per mic every 10ms (16kHz)
 *
 * @param data Buffer containing 4 times 160 samples. The samples are sorted by mic
 * so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
 * @param num_samples Tells how many data we get in total (should always be 640)
 */
void processAudioData(int16_t *data, uint16_t num_samples);

/**
 * @brief Puts the invoking thread into sleep until it can process the audio data.
 */
void wait_send_to_computer(void);

/**
 * @brief Returns the pointer to the BUFFER_NAME_t buffer asked.
 *
 * @param name Name of the buffer to get.
 * @return Pointer to the requested audio buffer.
 */
float *get_audio_buffer_ptr(BUFFER_NAME_t name);

/**
 * @brief Finds the highest peak in a buffer.
 *
 * @param buffer Buffer to find the highest peak from.
 * @param max_value Pointer to store the maximum value found.
 */
void find_highest_peak(float *buffer, float *max_value);

/**
 * @brief Calculates the average intensity of a buffer.
 *
 * @param buffer Buffer to calculate the average intensity from.
 * @param average_value Pointer to store the average intensity value.
 */
void calculate_average_intensity(float *buffer, float *average_value);

/**
 * @brief Gets the difference in intensity between the front-left and front-right microphones.
 *
 * @return Difference in intensity between front-left and front-right.
 */
float audio_get_diff_intensity_front_left(void);

/**
 * @brief Gets the difference in intensity between the front-right and front-left microphones.
 *
 * @return Difference in intensity between front-right and front-left.
 */
float audio_get_diff_intensity_front_right(void);

#endif /* AUDIO_PROCESSING_H */
