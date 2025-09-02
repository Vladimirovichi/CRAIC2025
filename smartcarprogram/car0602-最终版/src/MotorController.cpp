
#include "MotorController.h"
#include <iostream>

MotorController::MotorController(int pwmchip, int pwmnum, int gpioNum, unsigned int period_ns,
                                 double kp, double ki, double kd, double targetSpeed,
                                 int encoder_pwmNum, int encoder_gpioNum, int encoder_dir_)
    : pwmController(pwmchip, pwmnum), directionGPIO(gpioNum), pidController(kp, ki, kd, targetSpeed, INCREMENTAL, 80),
      encoder(encoder_pwmNum, encoder_gpioNum), encoder_dir(encoder_dir_)
{
    pwmController.setPeriod(period_ns); // 设置 PWM 周期
    directionGPIO.setDirection("out");
    pwmController.enable(); // 启用 PWM
}

MotorController::~MotorController(void)
{
    pwmController.disable();
}

void MotorController::updateduty(double dutyCycle)
{
    int newduty = pwmController.readPeriod() * abs(dutyCycle) / 100.0;
    if (newduty != pwmController.readDutyCycle())
    {
        pwmController.setDutyCycle(newduty);
    }

    // 根据 PID 输出设置 GPIO 的方向
    if (dutyCycle > 0)
    {
        directionGPIO.setValue(1); // 正向
    }
    else
    {
        directionGPIO.setValue(0); // 反向
    }
    //std::cout << encoder.pulse_counter_update() << std::endl;
}

void MotorController::updateSpeed(void)
{
    double encoderReading = encoder.pulse_counter_update() * encoder_dir;
    // std::cout << encoderReading << std::endl;
    double output = pidController.update(encoderReading);
    // int dutyCycle = static_cast<int>(output);

    // 设置 PWM 占空比
    updateduty(output);
    std::cout << encoderReading << "  " << output << std::endl;
}

void MotorController::updateTarget(int speed)
{
    pidController.setTarget(speed);
}
