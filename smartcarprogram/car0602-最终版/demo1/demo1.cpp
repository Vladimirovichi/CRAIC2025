
#include <iostream>
#include <unordered_set>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "MotorController.h"
#include "PwmController.h"
#include "GPIO.h"

GPIO mortorEN(73);
MotorController *motorController[2] = {nullptr, nullptr};
PwmController servo(1, 0);

void Init()
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
                                                 0, 0, 0, 0,
                                                 encoder_pwmchip[i], encoder_gpioNum[i], encoder_dir[i]);
    }

    servo.setPeriod(3000000);
    servo.setDutyCycle(1500000);
    servo.enable();
}

// 设置非阻塞输入
void setNonBlockingInput(bool enable)
{
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if (enable)
    {
        tty.c_lflag &= ~ICANON; // 禁用规范模式
        tty.c_lflag &= ~ECHO;   // 禁用回显
    }
    else
    {
        tty.c_lflag |= ICANON;
        tty.c_lflag |= ECHO;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

int main()
{
    std::unordered_set<char> pressedKeys;

    setNonBlockingInput(true);
    std::cout << "Press keys (WASD). Press 'q' to quit." << std::endl;

    Init();

    while (true)
    {
        char ch = getchar();

        if (ch == 'q')
        { // 按 'q' 退出
            break;
        }

        // 添加按下的键到集合中，并响应按下的键
        if (ch == 'w' || ch == 'W')
        {
            pressedKeys.insert('W');
            motorController[0]->updateduty(17);
            motorController[1]->updateduty(17);
            servo.setDutyCycle(1520000);
        }
        else if (ch == 'a' || ch == 'A')
        {
            pressedKeys.insert('A');
            servo.setDutyCycle(1360000);
            //servo.setDutyCycle(1560000);//这个角度应该会比较大 不对 修改过后舵机的转向反而变小了
            //设置转向的功能 转向角度
            //servo.setDutyCycle(1000000);//事实证明 数值越小 舵机的转向角度越大 这是一个切入口
        }
        else if (ch == 's' || ch == 'S')
        {
            pressedKeys.insert('S');
            motorController[0]->updateduty(-17);
            motorController[1]->updateduty(-17);
            servo.setDutyCycle(1520000);
        }
        else if (ch == 'd' || ch == 'D')
        {
            pressedKeys.insert('D');
            servo.setDutyCycle(1680000);
        }

        // 显示当前按下的键
        std::cout << "Pressed Keys: ";
        for (char key : pressedKeys)
        {
            std::cout << key << " ";
        }
        std::cout << std::endl;

        // 清空按下的键集合
        pressedKeys.clear();

        usleep(10000); // 10ms
    }

    setNonBlockingInput(false);
    return 0;
}