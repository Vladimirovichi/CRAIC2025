
#include "control.h"
#include "camera.h"
#include "GPIO.h"
//-------------------
#include <iostream>

#include "MotorController.h"
#include "global.h"
//---------------------------

MotorController *motorController[2] = {nullptr, nullptr};
GPIO mortorEN(73);
double mortor_kp = 1000;
double mortor_ki = 300;
double mortor_kd = 0;

void ControlInit()
{
    mortorEN.setDirection("out");
    mortorEN.setValue(1);
    const int pwmchip[2] = {8, 8};
    const int pwmnum[2] = {2, 1};
    const int gpioNum[2] = {12, 13};
    const int encoder_pwmchip[2] = {0, 3};
    const int encoder_gpioNum[2] = {75, 72};
    const int encoder_dir[2] = {1, -1};
    const unsigned int period_ns = 50000; // 20 kHz

    for (int i = 0; i < 2; ++i)
    {
        motorController[i] = new MotorController(pwmchip[i], pwmnum[i], gpioNum[i], period_ns,
                                                 mortor_kp, mortor_ki, mortor_kd, 0,
                                                 encoder_pwmchip[i], encoder_gpioNum[i], encoder_dir[i]);
    }
}

//发现了一处更改的地方
void ControlMain()
{
    //if (readFlag(start_file))  //加入了斑马线检测逻辑
    if (readFlag(start_file) && !zebra_signal) //目前表示没有检测到斑马线
    {
        //大致逻辑是 先触发语音模块  然后才标记标志位
        for (int i = 0; i < 2; ++i)
        {
            motorController[i]->updateduty(target_speed);

            // motorController[i]->pidController.setPID(mortor_kp, mortor_ki, mortor_kd);
            // motorController[i]->updateTarget(target_speed);
            // motorController[i]->updateSpeed();
        }
        mortorEN.setValue(1);
    }
    else//检测到了人行道斑马线 将两个电机的目标速度设置为0
    {
        for (int i = 0; i < 2; ++i)
        {
            motorController[i]->updateduty(0);
        }
        mortorEN.setValue(0);
    }
    return;
}

void ControlExit()
{
    for (int i = 0; i < 2; ++i)
    {
        delete motorController[i];
        std::cout << "motor" << i << "deleted\n";
    }
    mortorEN.setValue(0);
}
