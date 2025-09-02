#ifndef CONTROL_H
#define CONTROL_H

#include <unistd.h>

#include "MotorController.h"
#include "global.h"
#include "serial.h"

void ControlInit();
void ControlMain();
void ControlExit();

extern double mortor_kp;
extern double mortor_ki;
extern double mortor_kd;

#endif