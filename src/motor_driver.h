/**
 * @file     motor_driver.h
 * @brief    This header file contains function declarations for controlling motors.
**/

#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

#include <stdbool.h>

/**
 * @brief Start the PI regulator thread.
**/
void motor_regulator_start(void);

/**
 * @brief Get the status of the LEDS.
 * 
 * @return true if the LEDS are enabled, false otherwise.
**/
bool get_enabled_lights(void);

#endif /* PI_REGULATOR_H**/
