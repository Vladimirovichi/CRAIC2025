// #include "camera.h"
// #include <algorithm>
// #include <iomanip>
// #include <iostream>
// #include <mutex>
// #include <sstream>
// #include <thread>
// #include <chrono>
// #include "PIDController.h"
// #include "PwmController.h"
// #include "frame_buffer.h"
// #include "global.h"
// #include "vl53l0x.h"
// #include "wonderEcho.h"//头文件已引用

// cv::VideoCapture cap;

// double kp = 0;
// double ki = 0;
// double kd = 0;

// int screenWidth, screenHeight;
// int newWidth, newHeight;
// int fb;
// // 创建帧缓冲区
// uint16_t *fb_buffer;
// PwmController servo(1, 0);

// //补充
// bool zebra_mark = 0;

// static auto lastZebraTime = std::chrono::steady_clock::now();
// static auto lastRangeTime = std::chrono::steady_clock::now();
// static auto lastCrossTime = std::chrono::steady_clock::now();
// constexpr auto DEBOUNCE_INTERVAL = std::chrono::milliseconds(500); // 1000ms防抖间隔  数值改为500
// //目前的核心逻辑不避障即可

// std::atomic<bool> zebra_signal(false);//cpp文件中初始化
// std::atomic<bool> thread_running(false);
// std::chrono::steady_clock::time_point signal_end_time;
// #define calc_scale 2

// void signal_thread_func() {
//     // 设置信号激活状态
//     zebra_signal = true;//调用
//     wonderEchoSend(0xFF, 0x10); //
//     // 保持信号3秒
//     std::this_thread::sleep_for(std::chrono::seconds(3));//
    
//     // 关闭信号
//     zebra_signal = false;//调用
    
//     // 额外等待1秒冷却期
//     std::this_thread::sleep_for(std::chrono::seconds(1));
    
//     // 标记线程结束
//     thread_running = false;
// }
// //

// int CameraInit(uint8_t camera_id, double dest_fps, int width, int height)
// {
//     servo.setPeriod(3000000);
//     servo.setDutyCycle(servo_mid);
//     servo.enable();

//     // 打开帧缓冲区设备
//     fb = open("/dev/fb0", O_RDWR);
//     if (fb == -1)
//     {
//         std::cerr << "无法打开帧缓冲区设备" << std::endl;
//         return -1;
//     }

//     // 获取帧缓冲区设备信息
//     struct fb_var_screeninfo vinfo;
//     if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == -1)
//     {
//         std::cerr << "无法获取帧缓冲区信息" << std::endl;
//         close(fb);
//         return -1;
//     }

//     // 动态设置屏幕分辨率
//     screenWidth = vinfo.xres;
//     screenHeight = vinfo.yres;

//     // 计算帧缓冲区大小
//     size_t fb_size = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;

//     // 使用 mmap 映射帧缓冲区到内存
//     fb_buffer = (uint16_t *)mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
//     if (fb_buffer == MAP_FAILED)
//     {
//         std::cerr << "无法映射帧缓冲区到内存" << std::endl;
//         close(fb);
//         return -1;
//     }

//     // 打开默认摄像头（设备编号 0）
//     cap.open(camera_id);

//     // 检查摄像头是否成功打开
//     if (!cap.isOpened())
//     {
//         printf("无法打开摄像头\n");
//         munmap(fb_buffer, fb_size);
//         close(fb);
//         return -1;
//     }

//     cap.set(cv::CAP_PROP_FRAME_WIDTH, width);                                  // 宽度
//     cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);                                // 高度
//     cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); // 视频流格式
//     cap.set(cv::CAP_PROP_AUTO_EXPOSURE, -1);                                   // 设置自动曝光

//     // 获取摄像头实际分辨率
//     int cameraWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
//     int cameraHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
//     printf("摄像头分辨率: %d x %d\n", cameraWidth, cameraHeight);

//     // 计算 newWidth 和 newHeight，确保图像适应屏幕
//     double widthRatio = static_cast<double>(screenWidth) / cameraWidth;
//     double heightRatio = static_cast<double>(screenHeight) / cameraHeight;
//     double scale = std::min(widthRatio, heightRatio); // 选择较小的比例，确保图像不超出屏幕

//     newWidth = static_cast<int>(cameraWidth * scale);
//     newHeight = static_cast<int>(cameraHeight * scale);
//     printf("自适应分辨率: %d x %d\n", newWidth, newHeight);

//     // 计算帧率
//     double fps = cap.get(cv::CAP_PROP_FPS);
//     printf("Camera fps:%lf\n", fps);

//     line_tracking_width = newWidth / calc_scale;
//     line_tracking_height = newHeight / calc_scale;
//     //printf("巡线分辨率: %d x %d\n", line_tracking_width, line_tracking_height);  补充的代码
//     // 计算每帧的延迟时间（ms）
//     return static_cast<int>(1000.0 / std::min(fps, dest_fps));
// }


// void cameraDeInit(void)
// {
//     cap.release();

//     // 获取帧缓冲区设备信息
//     struct fb_var_screeninfo vinfo;
//     if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == -1)
//     {
//         std::cerr << "无法获取帧缓冲区信息" << std::endl;
//     }
//     else
//     {
//         // 计算帧缓冲区大小
//         size_t fb_size = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;

//         // 取消映射
//         munmap(fb_buffer, fb_size);
//     }

//     close(fb);
// }

// int saved_frame_count = 0;
// bool saveCameraImage(cv::Mat frame, const std::string &directory)
// {
//     if (frame.empty())
//     {
//         std::cerr << "Save Error: Frame is empty." << std::endl;
//         return 0;
//     }

//     // 构建文件名
//     std::ostringstream filename;
//     filename << directory << "/image_" << std::setw(5) << std::setfill('0') << saved_frame_count << ".jpg";

//     saved_frame_count++;
//     // 保存图像
//     return cv::imwrite(filename.str(), frame);
// }

// std::mutex frameMutex;
// cv::Mat pubframe;
// bool streamCaptureRunning;
// void streamCapture(void)
// {
//     cv::Mat frame;
//     while (streamCaptureRunning)
//     {
//         cap.read(frame);
//         frameMutex.lock();
//         pubframe = frame;
//         frameMutex.unlock();
//     }
//     return;
// }

// PIDController ServoControl(1.5, 1.0, 2.0, 0.0, POSITION, 1250000);
// int CameraHandler(void)
// {
//     cv::Mat resizedFrame;

//     frameMutex.lock();
//     raw_frame = pubframe;
//     frameMutex.unlock();

//     if (raw_frame.empty()) // 检查是否成功捕获图像
//     {
//         //printf("无法捕获图像\n");
//         return -1;
//     }

//     if (readFlag(saveImg_file))
//     {
//         if (saveCameraImage(raw_frame, "./image"))
//         {
//             printf("图像%d已保存\n", saved_frame_count);
//         }
//         else
//         {
//             printf("图像保存失败\n");
//             return -1;
//         }
//     }

//     { // 图像计算
//         image_main();
//         if (zebraLineDetected) {//检测到了斑马线
//             wonderEchoSend(0xFF, 0x10);//我只要检测到我就播报
//             if (!zebra_mark && !thread_running) {
//                 auto currentZebraTime = std::chrono::steady_clock::now();
                
//                 // 检查是否超过防抖间隔
//                 if (currentZebraTime - lastZebraTime >= DEBOUNCE_INTERVAL) {
//                     zebra_mark = 1;
//                     lastZebraTime = currentZebraTime; // 更新最后触发时间
            
//                     // 启动信号线程
//                     thread_running = true;//启动线程 标记着进行了停车  这个线程就是专门服务于人行道停车功能的
//                     std::thread(signal_thread_func).detach();//调用了线程
//                 }
//             }
//         } else {
//             zebra_mark = 0;
//         }
//     }

//     if (readFlag(start_file))//表示读取的文件数值为1 时 可以前进 其他情况下 不动  单纯表示当start为1时进行的操作
//     {
//         int foresee = readDoubleFromFile(foresee_file);//前瞻性
//         if (mid_line_filtered[foresee] != 255 )//正常行驶的情况
//         {
//             ServoControl.setPID(kp, ki, kd);
//             double servoduty = -ServoControl.update(mid_line_filtered[foresee / calc_scale] * calc_scale - newWidth / 2);
//             // std::cout << PID_rotate.load() << std::endl;
//             servoduty = std::clamp(servoduty, -6.0, 4.0);
//             double servoduty_ns = (servoduty) / 100 * servo.readPeriod() + servo_mid;//1520000;//后来是servo_mid//固定的数值
//             servo.setDutyCycle(servoduty_ns);//自己设置舵机转向周期 
//         }
//     }
//     else
//     {

//     }

//     // 显示图片
//     if (readFlag(showImg_file))
//      {
//           cv::Mat fbImage(screenHeight, screenWidth, CV_8UC3, cv::Scalar(0, 0, 0));

//           // 缩放视频到新尺寸
//           cv::resize(track, resizedFrame, cv::Size(newWidth, newHeight));
//     //     // 将单通道的二值化图像转换为三通道的彩色图像
//           cv::Mat coloredResizedFrame;
//           cv::cvtColor(resizedFrame, coloredResizedFrame, cv::COLOR_GRAY2BGR); // 转换为彩色图像

//     //     // 将缩放后的图像居中放置在帧缓冲区图像中，填充黑色边框
//           fbImage.setTo(cv::Scalar(0, 0, 0)); // 清空缓冲区（填充黑色）
//           cv::Rect roi((screenWidth - newWidth) / 2, (screenHeight - newHeight) / 2, newWidth, newHeight);
//           coloredResizedFrame.copyTo(fbImage(roi));

//     //     // 绘制左右边界线和中线
//           int scaledLeftX, scaledRightX, scaledMidX, scaledY;

//           for (int y = 0; y < line_tracking_height; y++)
//           {
//                // 根据缩放比例调整X坐标
//               scaledLeftX = static_cast<int>(left_line[y] * calc_scale);
//               scaledRightX = static_cast<int>(right_line[y] * calc_scale);
//               scaledMidX = static_cast<int>(mid_line[y] * calc_scale);
//               scaledY = static_cast<int>(y * calc_scale);

//              // 绘制左边界（红）
//               cv::line(fbImage(roi), cv::Point(scaledLeftX, scaledY), cv::Point(scaledLeftX, scaledY), cv::Scalar(0, 0, 255), calc_scale);
//              // 绘制右边界（绿）
//              cv::line(fbImage(roi), cv::Point(scaledRightX, scaledY), cv::Point(scaledRightX, scaledY), cv::Scalar(0, 255, 0), calc_scale);
//             // 绘制中线  （蓝）
//              cv::line(fbImage(roi), cv::Point(scaledMidX, scaledY), cv::Point(scaledMidX, scaledY), cv::Scalar(255, 0, 0), calc_scale);
//           }

//          // 将帧缓冲区图像转换为RGB565格式
//          convertMatToRGB565(fbImage, fb_buffer, screenWidth, screenHeight);
//       }
//     return 0;
// }




#include "camera.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <chrono>
#include "PIDController.h"
#include "PwmController.h"
#include "frame_buffer.h"
#include "global.h"
#include "vl53l0x.h"
#include "wonderEcho.h"//头文件已引用

cv::VideoCapture cap;

double kp = 0;
double ki = 0;
double kd = 0;

// === 右边线过左侧 → 左补偿（160×120 版） ===
constexpr int    RIGHT_LINE_LEFT_X = 22;   // 触发阈值 (像素)
constexpr double LEFT_BIAS_PX      = 0.0;  // 左拨补偿量 (像素)
const     cv::Scalar YELLOW(0,255,255);    // 画黄点
// =========================================

int screenWidth, screenHeight;
int newWidth, newHeight;
int fb;
// 创建帧缓冲区
uint16_t *fb_buffer;
PwmController servo(1, 0);

//补充
bool zebra_mark = 0;

static auto lastZebraTime = std::chrono::steady_clock::now();
static auto lastRangeTime = std::chrono::steady_clock::now();
static auto lastCrossTime = std::chrono::steady_clock::now();
constexpr auto DEBOUNCE_INTERVAL = std::chrono::milliseconds(500); // 1000ms防抖间隔  数值改为500
//目前的核心逻辑不避障即可

std::atomic<bool> zebra_signal(false);//cpp文件中初始化
std::atomic<bool> thread_running(false);
std::chrono::steady_clock::time_point signal_end_time;
#define calc_scale 2

void signal_thread_func() {
    // 设置信号激活状态
    zebra_signal = true;//调用
    wonderEchoSend(0xFF, 0x10); //
    // 保持信号3秒
    std::this_thread::sleep_for(std::chrono::seconds(3));//
    
    // 关闭信号
    zebra_signal = false;//调用
    
    // 额外等待1秒冷却期
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 标记线程结束
    thread_running = false;
}
//

int CameraInit(uint8_t camera_id, double dest_fps, int width, int height)
{
    servo.setPeriod(3000000);
    servo.setDutyCycle(servo_mid);
    servo.enable();

    // 打开帧缓冲区设备
    fb = open("/dev/fb0", O_RDWR);
    if (fb == -1)
    {
        std::cerr << "无法打开帧缓冲区设备" << std::endl;
        return -1;
    }

    // 获取帧缓冲区设备信息
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        std::cerr << "无法获取帧缓冲区信息" << std::endl;
        close(fb);
        return -1;
    }

    // 动态设置屏幕分辨率
    screenWidth = vinfo.xres;
    screenHeight = vinfo.yres;

    // 计算帧缓冲区大小
    size_t fb_size = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;

    // 使用 mmap 映射帧缓冲区到内存
    fb_buffer = (uint16_t *)mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fb_buffer == MAP_FAILED)
    {
        std::cerr << "无法映射帧缓冲区到内存" << std::endl;
        close(fb);
        return -1;
    }

    // 打开默认摄像头（设备编号 0）
    cap.open(camera_id);

    // 检查摄像头是否成功打开
    if (!cap.isOpened())
    {
        printf("无法打开摄像头\n");
        munmap(fb_buffer, fb_size);
        close(fb);
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);                                  // 宽度
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);                                // 高度
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); // 视频流格式
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, -1);                                   // 设置自动曝光

    // 获取摄像头实际分辨率
    int cameraWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cameraHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("摄像头分辨率: %d x %d\n", cameraWidth, cameraHeight);

    // 计算 newWidth 和 newHeight，确保图像适应屏幕
    double widthRatio = static_cast<double>(screenWidth) / cameraWidth;
    double heightRatio = static_cast<double>(screenHeight) / cameraHeight;
    double scale = std::min(widthRatio, heightRatio); // 选择较小的比例，确保图像不超出屏幕

    newWidth = static_cast<int>(cameraWidth * scale);
    newHeight = static_cast<int>(cameraHeight * scale);
    printf("自适应分辨率: %d x %d\n", newWidth, newHeight);

    // 计算帧率
    double fps = cap.get(cv::CAP_PROP_FPS);
    printf("Camera fps:%lf\n", fps);

    line_tracking_width = newWidth / calc_scale;
    line_tracking_height = newHeight / calc_scale;
    //printf("巡线分辨率: %d x %d\n", line_tracking_width, line_tracking_height);  补充的代码
    // 计算每帧的延迟时间（ms）
    return static_cast<int>(1000.0 / std::min(fps, dest_fps));
}


void cameraDeInit(void)
{
    cap.release();

    // 获取帧缓冲区设备信息
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        std::cerr << "无法获取帧缓冲区信息" << std::endl;
    }
    else
    {
        // 计算帧缓冲区大小
        size_t fb_size = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;

        // 取消映射
        munmap(fb_buffer, fb_size);
    }

    close(fb);
}

int saved_frame_count = 0;
bool saveCameraImage(cv::Mat frame, const std::string &directory)
{
    if (frame.empty())
    {
        std::cerr << "Save Error: Frame is empty." << std::endl;
        return 0;
    }

    // 构建文件名
    std::ostringstream filename;
    filename << directory << "/image_" << std::setw(5) << std::setfill('0') << saved_frame_count << ".jpg";

    saved_frame_count++;
    // 保存图像
    return cv::imwrite(filename.str(), frame);
}

std::mutex frameMutex;
cv::Mat pubframe;
bool streamCaptureRunning;
void streamCapture(void)
{
    cv::Mat frame;
    while (streamCaptureRunning)
    {
        cap.read(frame);
        frameMutex.lock();
        pubframe = frame;
        frameMutex.unlock();
    }
    return;
}

PIDController ServoControl(3.5, 0.8, 3.5, 0.0, POSITION, 1250000);
int CameraHandler(void)
{
    cv::Mat resizedFrame;

    frameMutex.lock();
    raw_frame = pubframe;
    frameMutex.unlock();

    if (raw_frame.empty()) // 检查是否成功捕获图像
    {
        //printf("无法捕获图像\n");
        return -1;
    }

    if (readFlag(saveImg_file))
    {
        if (saveCameraImage(raw_frame, "./image"))
        {
            printf("图像%d已保存\n", saved_frame_count);
        }
        else
        {
            printf("图像保存失败\n");
            return -1;
        }
    }

    { // 图像计算
        image_main();
        if (zebraLineDetected) {//检测到了斑马线
            wonderEchoSend(0xFF, 0x10);//我只要检测到我就播报
            if (!zebra_mark && !thread_running) {
                auto currentZebraTime = std::chrono::steady_clock::now();
                
                // 检查是否超过防抖间隔
                if (currentZebraTime - lastZebraTime >= DEBOUNCE_INTERVAL) {
                    zebra_mark = 1;
                    lastZebraTime = currentZebraTime; // 更新最后触发时间
            
                    // 启动信号线程
                    thread_running = true;//启动线程 标记着进行了停车  这个线程就是专门服务于人行道停车功能的
                    std::thread(signal_thread_func).detach();//调用了线程
                }
            }
        } else {
            zebra_mark = 0;
        }
    }
    int foresee = readDoubleFromFile(foresee_file);//前瞻性
    int rowIdx = std::clamp(foresee / calc_scale, 0, line_tracking_height-1);
    if (readFlag(start_file))//表示读取的文件数值为1 时 可以前进 其他情况下 不动  单纯表示当start为1时进行的操作
    {

        if (mid_line_filtered[rowIdx] != 255 )//正常行驶的情况
        {
            ServoControl.setPID(kp, ki, kd);
            double servoduty = -ServoControl.update(mid_line_filtered[foresee / calc_scale] * calc_scale - newWidth / 2);
            /* ---------- ② 右线检测 & 左偏 ---------- */
            if (left_line_filtered[rowIdx] != 255)                // 右边线存在
            {
                
                int rightX = left_line_filtered[rowIdx] * calc_scale; // 缩放回 0-159
                if (rightX < RIGHT_LINE_LEFT_X)                // “太靠左” → 触发
                {
                    servoduty -= (LEFT_BIAS_PX / newWidth) * 100.0; // 左补偿 %
                }
            }

            // std::cout << PID_rotate.load() << std::endl;
            servoduty = std::clamp(servoduty, -6.0, 5.0);
            double servoduty_ns = (servoduty) / 100 * servo.readPeriod() + servo_mid;//1520000;//后来是servo_mid//固定的数值
            servo.setDutyCycle(servoduty_ns);//自己设置舵机转向周期 
        }
    }
    else
    {

    }

    // 显示图片
    if (readFlag(showImg_file))
     {
          cv::Mat fbImage(screenHeight, screenWidth, CV_8UC3, cv::Scalar(0, 0, 0));

          // 缩放视频到新尺寸
          cv::resize(track, resizedFrame, cv::Size(newWidth, newHeight));
    //     // 将单通道的二值化图像转换为三通道的彩色图像
          cv::Mat coloredResizedFrame;
          cv::cvtColor(resizedFrame, coloredResizedFrame, cv::COLOR_GRAY2BGR); // 转换为彩色图像

    //     // 将缩放后的图像居中放置在帧缓冲区图像中，填充黑色边框
          fbImage.setTo(cv::Scalar(0, 0, 0)); // 清空缓冲区（填充黑色）
          cv::Rect roi((screenWidth - newWidth) / 2, (screenHeight - newHeight) / 2, newWidth, newHeight);
          coloredResizedFrame.copyTo(fbImage(roi));

    //     // 绘制左右边界线和中线
          int scaledLeftX, scaledRightX, scaledMidX, scaledY;

          for (int y = 0; y < line_tracking_height; y++)
          {
               // 根据缩放比例调整X坐标
              scaledLeftX = static_cast<int>(left_line_filtered[y] * calc_scale);
              scaledRightX = static_cast<int>(right_line_filtered[y] * calc_scale);
              scaledMidX = static_cast<int>(mid_line_filtered[y] * calc_scale);
              scaledY = static_cast<int>(y * calc_scale);

             // 绘制左边界（红）
              cv::line(fbImage(roi), cv::Point(scaledLeftX, scaledY), cv::Point(scaledLeftX, scaledY), cv::Scalar(0, 0, 255), calc_scale);
             // 绘制右边界（绿）
             cv::line(fbImage(roi), cv::Point(scaledRightX, scaledY), cv::Point(scaledRightX, scaledY), cv::Scalar(0, 255, 0), calc_scale);
            // 绘制中线  （蓝）
             cv::line(fbImage(roi), cv::Point(scaledMidX, scaledY), cv::Point(scaledMidX, scaledY), cv::Scalar(255, 0, 0), calc_scale);
          }
        /* ===== 高亮前瞻行右边线 (黄色) ===== */
        if (right_line_filtered[rowIdx] != 255)
        {
            int hlX = right_line_filtered[rowIdx] * calc_scale;
            int hlY = rowIdx * calc_scale;
            cv::line(fbImage(roi),
                    cv::Point(hlX, hlY),
                    cv::Point(hlX, hlY),
                    YELLOW, calc_scale);        // 2×2 黄点
        }
        /* =================================== */

         // 将帧缓冲区图像转换为RGB565格式
         convertMatToRGB565(fbImage, fb_buffer, screenWidth, screenHeight);
      }
    return 0;
}
