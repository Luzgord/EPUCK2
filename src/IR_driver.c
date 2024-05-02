//----stdlib includes----
#include <stdlib.h>

//----global chibiOS includes----
#include <ch.h>
#include <hal.h>

//----specific epuck2 includes----          Lesquels de ces includes sont vraiment utiles ?
#include <sensors\proximity.h>
#include <msgbus\messagebus.h>
#include <i2c_bus.h>
/*
	 IR sensors on robot
	 	   FRONT
    --------------------
        _ _ - - _ _
    - - \       /   - -
  -     IR8    IR1      -
 - \                  /  -
-  IR7              IR2   -
-                          -
--IR6                 IR3--
-                          -
 -                       -
  -  IR5         IR4    -
    - /           \  - -
      - -  _ _ -  -
    --------------------
           BACK
*/
static THD_WORKING_AREA(waIR, 256);
static THD_FUNCTION(IR, arg) {
    
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1) {
        
    }
}

void IR_start(void) {
    chThdCreateStatic(waIR, sizeof(waIR), NORMALPRIO, IR, NULL);
}