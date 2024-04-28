#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H
//mettre ces define dici ou dans main.h?
#define KP						800.0f
#define KI 						3.5f
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2 
#define ERROR_THRESHOLD			0.1f

//start the PI regulator thread
void pi_regulator_start(void);

//handle motors
void set_enabled_motors(bool enable);
void toogle_enabled_motors(void);

#endif /* PI_REGULATOR_H */