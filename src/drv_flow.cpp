/*
 * \file drv_flow.cpp
 * \程序驱动文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>

#include "Process.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    VideoCapture capture(0);
    int mode = 0;
    int counter = 0;
    int w = 640;
    int h = 480;

    switch (argc)
    {
    case 1:
        break;
    case 3:
        w = atoi(argv[1]);
        h = atoi(argv[2]);
        w = w % 16 == 0 ? w : (w / 16 + 1) * 16;
        h = h % 16 == 0 ? h : (h / 16 + 1) * 16;
        break;
    default:
        cout << "\n\n使用方式:   " << argv[0] << " （输出图片宽度 输出图片高度）\n";
        cout << "其中：宽度范围在0~1280，高度范围在0~720，默认值为640*480，自动调节为大等于输入值的最小的16的倍数。\n";
        cout << "识别模式下，图片像素值固定为640*320 \n";
        return 0;
    }

    while(1)
    {
        switch(mode)
        {
        case 0:
            capture.set(CAP_PROP_FRAME_WIDTH, 1280); // max:1280 
            capture.set(CAP_PROP_FRAME_HEIGHT, 720); // max:720 
            if (!capture.isOpened())
            {
                cerr << "open error \n" << endl;
                return 1;
            }
            counter = process_normal(capture, w, h, counter);
            break;
        case 1:
            capture.set(CAP_PROP_FRAME_WIDTH, 640); // max:1280 
            capture.set(CAP_PROP_FRAME_HEIGHT, 480); // max:720 
            if (!capture.isOpened())
            {
                cerr << "open error \n" << endl;
                return 1;
            }
            counter = process_recognition_picture(capture, counter);
            break;
        case 2:
            capture.set(CAP_PROP_FRAME_WIDTH, 640); // max:1280 
            capture.set(CAP_PROP_FRAME_HEIGHT, 480); // max:720 
            if (!capture.isOpened())
            {
                cerr << "open error \n" << endl;
                return 1;
            }
            counter = process_recognition_frequency(capture, counter);
            break;
        default:
            cout << "Wrong mode. Please try again." << endl;
        }

        char op;
        cout << "你要进行的操作?" << endl 
            << "（'q'：退出，'0'：普通模式，'1'：RGB图像识别模式，'2'：频域识别模式。）" << endl;
        cin >> op;
        switch(op)
        {
        case 'q':
            return 0;
        case '0':
            mode = 0;
            break;
        case '1':
            mode = 1;
            break;
        case '2':
            mode = 2;
            break;
        default:
            cout << "Wrong key. Please try again." << endl;
        }
    }
    return 0;
}