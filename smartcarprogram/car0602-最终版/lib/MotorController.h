
#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "PwmController.h"
#include "PIDController.h"
#include "GPIO.h"
#include "encoder.h"

class MotorController
{
public:
    MotorController(int pwmchip, int pwmnum, int gpioNum, unsigned int period_ns,
                    double kp, double ki, double kd, double targetSpeed,
                    int encoder_pwmNum, int encoder_gpioNum, int encoder_dir_);
    ~MotorController(void);

    void updateSpeed(void);
    void updateTarget(int speed);
    void updateduty(double dutyCycle);
    PIDController pidController;

private:
    PwmController pwmController;
    ENCODER encoder;
    GPIO directionGPIO;
    int encoder_dir;
};

#endif // MOTOR_CONTROLLER_H
