/**
 * @file     main.c
 * @brief    This source file contains the main function initiate all process.
**/

/*File from e-puck library*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <usbcfg.h>
#include <motors.h>
#include <audio/microphone.h>
#include <arm_math.h>
#include <spi_comm.h>
#include <leds.h>
#include <sensors/proximity.h>
/*Specific files*/
#include "main.h"
#include "audio_processing.h"
#include "fft.h"
#include "lights.h"
#include "motor_driver.h"

static void serial_start(void){
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void) {
   
    // Global initialization   
    halInit();
    chSysInit();
    mpu_init();   
    spi_comm_start();  // Starts the SPI communication for rgb leds 
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    // Starts the serial communication
    serial_start();    // UART3 for serial communication
    // usb_start();       // USB for serial communication

    // Starts specific threads or functions from the epuck2 library
    clear_leds();
    mic_start(&processAudioData);
    proximity_start();
    motors_init();

    // Start our threads   
    motor_regulator_start();
    lights_start(); 

    while (1) {
        chThdSleepMilliseconds(1000);
    }

}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void){
    chSysHalt("Stack smashing detected");
}
