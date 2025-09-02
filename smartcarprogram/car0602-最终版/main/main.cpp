#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string>
#include <csignal>
#include <atomic>
#include "image_cv.h"
#include "global.h"
#include "camera.h"
#include "control.h"
#include "Timer.h"
#include "serial.h"
#include "encoder.h"
#include "video.h"
//以下两行是新增加的demo
#include "wonderEcho.h"//头文件已引用


std::atomic<bool> running(true);
void signalHandler(int signal)
{
    running.store(false);
}

int main(void)
{
    std::signal(SIGINT, signalHandler);

    double dest_fps = readDoubleFromFile(destfps_file);
    int dest_frame_duration = CameraInit(0, dest_fps, 320, 240);
    //printf("%d\n", dest_frame_duration);
    if (dest_frame_duration != -1)
    {
        wonderEchoInit();
        streamCaptureRunning = true;
        std::thread camworker = std::thread(streamCapture);
        ControlInit();

        Timer CameraTimer(dest_frame_duration, std::bind(CameraHandler));
        Timer MortorTimer(8, std::bind(ControlMain));
        CameraTimer.start();
        MortorTimer.start();

        // 主循环，直到用户输入 Ctrl+C
        while (running.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            target_speed = readDoubleFromFile(speed_file);//循环读取数值
            servo_mid = readDoubleFromFile(servo_mid_file); //补充 

            mortor_kp = readDoubleFromFile(mortor_kp_file);
            mortor_ki = readDoubleFromFile(mortor_ki_file);
            mortor_kd = readDoubleFromFile(mortor_kd_file);

            kp = readDoubleFromFile(kp_file);
            ki = readDoubleFromFile(ki_file);
            kd = readDoubleFromFile(kd_file);
            // vofa_image(1, 160*120, 160, 120, Format_Grayscale8, (char*)IMG);
        }
        //std::cout << "Stopping!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        CameraTimer.stop();
        //std::cout << "Camera Timer stopped!\n";
        MortorTimer.stop();
        //std::cout << "Control Timer stopped!\n";

        ControlExit();
        //std::cout << "Control Service stopped!\n";

        streamCaptureRunning = false;
        if (camworker.joinable())
        {
            camworker.join();
        }

        cameraDeInit();
        //std::cout << "Camera Service stopped!\n";
    }
    return 0;
}