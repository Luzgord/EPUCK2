/**
 * @file     main.h
 * @brief    This header file contains important global variable from TP5.
**/

#ifndef MAIN_H
#define MAIN_H

#include <camera/dcmi_camera.h>
#include <msgbus/messagebus.h>
#include <parameter/parameter.h>

/* Robot wide IPC bus. */
extern messagebus_t bus;
/* Robot wide parameter tree. */
extern parameter_namespace_t parameter_root;

#endif