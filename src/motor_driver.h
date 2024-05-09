#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

#include <stdbool.h>

/**
 * @brief Start the PI regulator thread.
 */
void motor_regulator_start(void);

/**
 * @brief Get the status of the gyroscope.
 * 
 * @return true if the gyroscope is enabled, false otherwise.
 */
bool get_enabled_giro(void);

#endif /* PI_REGULATOR_H */
