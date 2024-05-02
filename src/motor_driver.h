#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H
#define KP						0.4f
#define KI 						0.0f
#define KD 						0.0f
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT) //KI //vérifier la cohérence de cette valeur avec assistant
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			1 
#define ERROR_THRESHOLD			1000 //Define by expermientation can be modified 
#define NO_CORRECTION			0
#define IR_FRONT_LEFT 			7
#define IR_FRONT_RIGHT 			0
#define MIN_DISTANCE_TO_WALL 	110 // e qual to aproximatively 4cm (value obtain emirically)

//start the PI regulator thread
void motor_regulator_start(void);
void wall_detetection(void);
//handle motors
void set_enabled_motors(bool enable);
void toogle_enabled_motors(void);
void print_IR_values(int IR_L, int IR_R);

#endif /* PI_REGULATOR_H */