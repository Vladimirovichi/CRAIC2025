
#ifndef CAMERA_H_
#define CAMERA_H_

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <algorithm>
#include <chrono>

#include "image_cv.h"
#include "PIDController.h"
#include "PwmController.h"
#include "global.h"
#include "frame_buffer.h"
#include "serial.h"

int CameraInit(uint8_t camera_id, double dest_fps, int width, int height);
int CameraHandler(void);
void streamCapture(void);
void cameraDeInit(void);

extern bool streamCaptureRunning;


extern std::atomic<bool> zebra_signal;//人行道标志位 全局变量定义

extern double kp;
extern double ki;
extern double kd;

#endif
//111