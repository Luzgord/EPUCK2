#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H
//mettre ces define dici ou dans main.h?
#define KP						0.4f
#define KI 						0.0f
#define KD 						0.0f
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT)//KI) //vérifier la cohérence de cette valeur avec assistant
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			1 
#define ERROR_THRESHOLD			1000 //Define by expermientation can be modified 
#define NO_CORRECTION			0

//start the PI regulator thread
void motor_regulator_start(void);

//handle motors
void set_enabled_motors(bool enable);
void toogle_enabled_motors(void);

#endif /* PI_REGULATOR_H */