
#ifndef PWM_CONTROLLER_H
#define PWM_CONTROLLER_H

#include <string>
#include <fstream>
#include <iostream>

class PwmController
{
public:
    PwmController(int pwmchip, int pwmnum, bool polarity = true);
    ~PwmController();

    bool enable();
    bool disable();
    bool setPeriod(unsigned int period_ns);
    bool setDutyCycle(unsigned int duty_cycle_ns);
    bool setPolarity(bool polarity);
    bool initialize();
    int readPeriod();
    int readDutyCycle();

private:
    std::string pwmPath; // PWM设备的路径
    int pwmchip;
    int pwmnum;
    int period;
    int duty_cycle;
    bool writeToFile(const std::string &path, const std::string &value);
};

#endif
