
#include "image_cv.h"
#include <iostream>
#include "global.h"
#include <vector>
#include <deque>
#include <cmath>

#include <fcntl.h>
#include <linux/fb.h>
#include <opencv2/opencv.hpp>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <numeric>

cv::Mat raw_frame;
cv::Mat grayFrame;
cv::Mat binarizedFrame;
cv::Mat morphologyExFrame;
cv::Mat track;

std::vector<int> left_line;              // 左边缘列号数组
std::vector<int> right_line;             // 右边缘列号数组
std::vector<int> mid_line;               // 中线列号数组
std::vector<double> left_line_filtered;  // 中线列号数组
std::vector<double> right_line_filtered; // 中线列号数组
std::vector<double> mid_line_filtered;   // 中线列号数组

int line_tracking_width;
int line_tracking_height;

bool zebraLineDetected = false;

//斑马线检测函数
bool detectZebraCrossing(const cv::Mat& gray) {
    const int height = gray.rows;
    const int width = gray.cols;
    const int scan_y = height - std::max(20, height/2);  // 检测区域Y坐标（底部向上）
    const int x_start = width * 0.1;                     // 检测区域左边界
    const int x_end = width * 0.9;                       // 检测区域右边界
    const int GRAY_THRESHOLD = 160;                      // 灰度阈值
    const int MIN_TRANSITIONS = 10;                      // 最小跳变次数
    const int scan_lines = 5;                            // 扫描线数量

    // 计算灰度跳变次数
    int total_transitions = 0;
    for (int i = 0; i < scan_lines; ++i) {
        int current_y = scan_y - i * 5;
        bool current_white = gray.at<uchar>(current_y, x_start) > GRAY_THRESHOLD;
        int line_transitions = 0;

        for (int x = x_start; x < x_end; x += 3) {
            bool pixel_white = gray.at<uchar>(current_y, x) > GRAY_THRESHOLD;
            if (pixel_white != current_white) {
                line_transitions++;
                current_white = pixel_white;
            }
        }
        total_transitions += line_transitions;
    }

    // 判断是否为斑马线（平均跳变次数达标）
    return (total_transitions/scan_lines >= MIN_TRANSITIONS);
}


cv::Mat image_binerize(cv::Mat &frame)
{
    cv::Mat output;
    cv::Mat binarizedFrame;
    cv::Mat hsvImage;
    cv::cvtColor(frame, hsvImage, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvImage, hsvChannels);

    cv::threshold(hsvChannels[0], binarizedFrame, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    cv::threshold(hsvChannels[1], output, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    cv::bitwise_or(output, binarizedFrame, output);

    return output;
}

// cv::Mat image_binerize(cv::Mat &frame)
// {
//     cv::Mat hsvImage, mask;
//     cv::cvtColor(frame, hsvImage, cv::COLOR_BGR2HSV);

//     // 以下阈值变量请你根据实际赛道颜色调整
//     // 例如黄色范围
//     int h_min = 60;  // 你自己设置
//     int h_max = 145;
//     int s_min = 10;
//     int s_max = 25;
//     int v_min = 151;
//     int v_max = 255;

//     cv::Scalar lowerHSV(h_min, s_min, v_min);
//     cv::Scalar upperHSV(h_max, s_max, v_max);

//     cv::inRange(hsvImage, lowerHSV, upperHSV, mask);

//     return mask;
// }

cv::Mat find_road(cv::Mat &frame)
{
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(2, 2));
    cv::morphologyEx(binarizedFrame, morphologyExFrame, cv::MORPH_OPEN, kernel);

    cv::Mat mask = cv::Mat::zeros(line_tracking_height + 2, line_tracking_width + 2, CV_8UC1);

    cv::Point seedPoint(line_tracking_width / 2, line_tracking_height - 10);

    cv::circle(morphologyExFrame, seedPoint, 10, 255, -1);

    cv::Scalar newVal(128);

    cv::Scalar loDiff = cv::Scalar(20);
    cv::Scalar upDiff = cv::Scalar(20);

    cv::floodFill(morphologyExFrame, mask, seedPoint, newVal, 0, loDiff, upDiff, 8);

    cv::Mat outputImage = cv::Mat::zeros(line_tracking_width, line_tracking_height, CV_8UC1);

    mask(cv::Rect(1, 1, line_tracking_width, line_tracking_height)).copyTo(outputImage);

    return outputImage;
}


void image_main()
{
    cv::Mat resizedFrame;

    cv::resize(raw_frame, resizedFrame, cv::Size(line_tracking_width, line_tracking_height));

    cv::cvtColor(raw_frame, grayFrame, cv::COLOR_BGR2GRAY);
    binarizedFrame = image_binerize(resizedFrame);

    track = find_road(binarizedFrame);

    left_line.clear();
    right_line.clear();
    mid_line.clear();
    left_line_filtered.clear();
    right_line_filtered.clear();
    mid_line_filtered.clear();

    left_line.resize(line_tracking_height, -1);
    right_line.resize(line_tracking_height, -1);
    mid_line.resize(line_tracking_height, -1);
    left_line_filtered.resize(line_tracking_height, -1);
    right_line_filtered.resize(line_tracking_height, -1);
    mid_line_filtered.resize(line_tracking_height, -1);

    uchar(*IMG)[line_tracking_width] = reinterpret_cast<uchar(*)[line_tracking_width]>(track.data);

    //在这里处理斑马线检测逻辑 直接调用斑马线检测函数 
    if(detectZebraCrossing(grayFrame)) {
        zebraLineDetected = true;//代码逻辑不符合要求 传入的应该是灰度图 而不是彩色图像
    }
    else {
        zebraLineDetected = false;
    }

    for (int i = 0; i < line_tracking_height; ++i)
    {
        int max_start = -1;
        int max_end = -1;
        int current_start = -1;
        int current_length = 0;
        int max_length = 0;

        for (int j = 0; j < line_tracking_width; ++j)
        {
            if (IMG[i][j])
            {
                if (current_length == 0)
                {
                    current_start = j;
                    current_length = 1;
                }
                else
                {
                    current_length++;
                }
                if (current_length >= max_length)
                {
                    max_length = current_length;
                    max_start = current_start;
                    max_end = j;
                }
            }
            else
            {
                current_length = 0;
                current_start = -1;
            }
        }
        if (max_length > 0)
        {
            left_line[i] = max_start;
            right_line[i] = max_end;
        }
        else
        {
            left_line[i] = -1;
            right_line[i] = -1;
        }
    }

    double a = 0.3;
    for (int row = line_tracking_height - 1; row >= 10; --row)
    {
        if (left_line[row] == -1 && right_line[row] == -1)
        {
            mid_line[row] = mid_line[row + 1];
            if (mid_line[row] > line_tracking_width / 2)
            {
                right_line[row] = line_tracking_width - 1;
                left_line[row] = mid_line[row + 1];
            }
            else
            {
                left_line[row] = 0;
                right_line[row] = mid_line[row + 1];
            }
        }
        else
        {
            mid_line[row] = (left_line[row] + right_line[row]) / 2;
        }
        if (row == line_tracking_height - 1)
        {
            left_line_filtered[row] = left_line[row];
            right_line_filtered[row] = right_line[row];
            mid_line_filtered[row] = mid_line[row];
        }
        else
        {
            left_line_filtered[row] = a * left_line[row] + (1 - a) * left_line_filtered[row + 1];
            right_line_filtered[row] = a * right_line[row] + (1 - a) * right_line_filtered[row + 1];
            // mid_line_filtered[row] = a * mid_line[row] + (1 - a) * mid_line_filtered[row + 1];
            mid_line_filtered[row] = (left_line_filtered[row] + right_line_filtered[row]) / 2.0;
        }
    }
    
}