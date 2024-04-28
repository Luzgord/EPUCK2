#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <arm_math.h>
#include <spi_comm.h>
#include <leds.h>

#include "main.h"
#include "audio_processing.h"
#include "fft.h"
#include "communications.h"
#include "siren.h"
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

int main(void) {
    halInit();
    chSysInit();
    mpu_init();
    //starts the USB communication
    usb_start();
    //starts the serial communication
    serial_start();
    //starts the SPI communication for rgb leds 
    spi_comm_start();
    // timer12_start();
    //inits the leds
    clear_leds();

    //starts the microphones processing thread
    mic_start(&processAudioData);

    //starts the siren thread
    siren_start();

    //starts the motors thread
    motors_init();
    motor_regulator_start();

    //starts the communication thread
    // comms_start();

    while (1) {
        chThdSleepMilliseconds(1000);
    }

}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void){
    chSysHalt("Stack smashing detected");
}
