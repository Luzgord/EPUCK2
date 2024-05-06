#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

#define KP						0.3f
#define KD 						1000.0f
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			1 
#define ERROR_THRESHOLD			1000
#define NO_CORRECTION			0
#define IR_FRONT_LEFT 			7
#define IR_FRONT_RIGHT 			0
#define MIN_DISTANCE_TO_WALL 	110 //equal to approximately 4cm
#define SIREN_LOOPS             5
#define SIREN_HFREQ             150
#define SIREN_LFREQ             100
#define CONST_SPEED             500

//start the PI regulator thread
void motor_regulator_start(void);

int16_t pd_regulator(float intensity_gap);

bool wall_detection(void);

#endif /* PI_REGULATOR_H */