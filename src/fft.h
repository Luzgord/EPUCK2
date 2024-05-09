/**
 * @file     fft.h
 * @brief    This header file contains function declarations for performing Fast Fourier Transform (FFT).
**/

#ifndef FFT_H
#define FFT_H

// Structure to represent a complex number with floating point components
typedef struct complex_float{
    float real;  
    float imag;  
}complex_float;

/**
 * @brief Function to perform an optimized FFT on a buffer of complex numbers
 * @param size The number of elements in the buffer
 * @param complex_buffer A pointer to the first element of the buffer
**/
void doFFT_optimized(uint16_t size, float* complex_buffer);

/**
 * @brief Function to perform a non-optimized (canonical) FFT on a buffer of complex numbers
 * @param size The number of elements in the buffer
 * @param complex_buffer A pointer to the first element of the buffer
**/
void doFFT_c(uint16_t size, complex_float* complex_buffer);

#endif /* FFT_H */