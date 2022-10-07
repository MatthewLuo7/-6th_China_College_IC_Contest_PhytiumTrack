/*
 * \file Process.h
 * \程序运行模式头文件
 */

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>
#include <time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>
#include <MNN/Interpreter.hpp>

#include "Encode.h"
#include "Decode.h"

using namespace std;
using namespace cv;
using namespace MNN;

typedef Vec<float, 32> Vec32f;

struct bbox_info
{
    int bbox_index;
    float bbox_confidence;
    float x1;
    float x2;
    float y1;
    float y2;
    bbox_info* last;
    bbox_info* next;
    bbox_info(): bbox_index(0), bbox_confidence(0.0), x1(0.0), x2(0.0), y1(0.0), y2(0.0), last(NULL), next(NULL) {};
};

void NMS(float* inf_out_t, Mat& bitmap, int mode);

int process_normal(VideoCapture& capture, int w, int h, int counter);

int process_recognition_picture(VideoCapture& capture, int counter);

int process_recognition_frequency(VideoCapture& capture, int counter);