/*
 * \file Process.cpp
 * \程序运行模式主体
 */

#include "Process.h"

Mat frame;
Rect frame_crop;
Mat frame_clone;
Mat M = Mat::zeros(40, 80, CV_32FC(32));

float sel_chan_mean[32] = { -82.07542, -0.06815056, 0.012199722, -0.04747111, -0.011168334, -0.021345556, 1.1102967, -0.0037752779, 0.009873888, 0.0005872222, 0.019345833, 0.0116880555, 0.022896666, -0.008367778, 0.059353888, -0.0012402778, -0.006374167, -0.0064433333, 0.029869445, 0.068339445, 
                            -42.21496, 0.0016793478, 0.0078108697, 0.55901086, 0.003978261, 0.028547825, 
                            -9.114439, -0.107481524, 0.02228913, -0.4159337, -0.004732609, -0.022843478};
float sel_chan_var_std[32] = { 391.66314697265625, 53.71498489379883, 29.647615432739258, 19.6500186920166, 14.668098449707031, 11.4932279586792, 70.50907135009766, 32.617305755615234, 21.780193328857422, 15.626815795898438, 39.97907638549805, 23.894481658935547, 17.492231369018555, 13.226285934448242, 26.816402435302734, 18.053796768188477, 14.180076599121094, 11.277414321899414, 20.220054626464844, 15.981060981750488, 
                            126.05424499511719, 14.047359466552734, 6.741161346435547, 18.35378074645996, 7.844638824462891, 8.998893737792969, 
                            99.74384307861328, 11.117897033691406, 5.2253241539001465, 16.23267936706543, 6.561285972595215, 7.95899772644043};

float Threshold = 0.1;

/*
void NMS(float* inf_out_t, Mat& bitmap, int mode)
{
    int counter = 0;
    int counter_2 = 0;
    struct bbox_info bbox_storage[200];
    struct bbox_info* p = NULL;

    for(int i = 0; i < 200; i++) //筛选所有可能的bound box
    {
        if(inf_out_t[6*i+4] > 0.02)
        {
            bbox_storage[counter].bbox_index = i;
            bbox_storage[counter].bbox_confidence = inf_out_t[6*i+4];
            counter++;
        }
    }

    if(counter)
    {
        if(mode)
        {
            for(int i = 0; i < counter; i++) //计算所有可能bound box的四角坐标
            {
                bbox_storage[i].x1 = (inf_out_t[bbox_storage[i].bbox_index*6+0] - (inf_out_t[bbox_storage[i].bbox_index*6+2]/2));
                bbox_storage[i].x2 = (inf_out_t[bbox_storage[i].bbox_index*6+0] + (inf_out_t[bbox_storage[i].bbox_index*6+2]/2));
                bbox_storage[i].y1 = (inf_out_t[bbox_storage[i].bbox_index*6+1] - (inf_out_t[bbox_storage[i].bbox_index*6+3]/2));
                bbox_storage[i].y2 = (inf_out_t[bbox_storage[i].bbox_index*6+1] + (inf_out_t[bbox_storage[i].bbox_index*6+3]/2));
            }
        }
        else
        {
            for(int i = 0; i < counter; i++) //计算所有可能bound box的四角坐标
            {
                bbox_storage[i].x1 = (inf_out_t[bbox_storage[i].bbox_index*6+0] - (inf_out_t[bbox_storage[i].bbox_index*6+2]/2)) * 2;
                bbox_storage[i].x2 = (inf_out_t[bbox_storage[i].bbox_index*6+0] + (inf_out_t[bbox_storage[i].bbox_index*6+2]/2)) * 2;
                bbox_storage[i].y1 = (inf_out_t[bbox_storage[i].bbox_index*6+1] - (inf_out_t[bbox_storage[i].bbox_index*6+3]/2)) * 2;
                bbox_storage[i].y2 = (inf_out_t[bbox_storage[i].bbox_index*6+1] + (inf_out_t[bbox_storage[i].bbox_index*6+3]/2)) * 2;
            }
        }
    }
    else
    {
        return;
    }

    counter_2 = counter;
    while(counter)
    {
        float SI = 0.0;
        float SU = 0.0;
        float IoU = 0.0;

        p = bbox_storage;
        for(int i = 0; i < counter_2; i++)
        {
            if(bbox_storage[i].bbox_confidence >= p -> bbox_confidence)
            {
                p = &bbox_storage[i];
            }
        }

        for(int i = 0; i < counter_2; i++)
        {
            if(&bbox_storage[i] != p)
            {
                if(p -> x2 < bbox_storage[i].x1||p -> x1 > bbox_storage[i].x2||p -> y2 < bbox_storage[i].y1||p -> y1 > bbox_storage[i].y2)
                {
                    SI = 0.0;
                }
                else
                {
                    float d_x = (p -> x2 < bbox_storage[i].x2 ? p -> x2 : bbox_storage[i].x2) - (p -> x1 > bbox_storage[i].x1 ? p -> x1 : bbox_storage[i].x1);
                    float d_y = (p -> y2 < bbox_storage[i].y2 ? p -> y2 : bbox_storage[i].y2) - (p -> y1 > bbox_storage[i].y1 ? p -> y1 : bbox_storage[i].y1);
                    SI = d_x * d_y;
                }
                SU =  (p -> x2 - p -> x1) * (p -> y2 - p -> y1) + (bbox_storage[i].x2 - bbox_storage[i].x1) * (bbox_storage[i].y2 - bbox_storage[i].y1) - SI;
                IoU = SI / SU;
                if(IoU > Threshold)
                {
                    bbox_storage[i].bbox_confidence = 0.0;
                    counter--;
                }
            }
        }

        int x1i, x2i, y1i, y2i = 0;
        x1i = (int) p -> x1;
        x2i = (int) p -> x2;
        y1i = (int) p -> y1;
        y2i = (int) p -> y2;
        rectangle(bitmap, Point(x1i, y1i), Point(x2i, y2i), Scalar( 0, 255, 255 ), 2, 8);

        p -> bbox_confidence = 0.0;
        counter--;
    }
}
*/

void NMS(float* inf_out_t, Mat& bitmap, int mode)
{
    int counter = 0;
    struct bbox_info* p = NULL;
    struct bbox_info* q = NULL;
    struct bbox_info* head = NULL;
    struct bbox_info* tail = NULL;

    for(int i = 0; i < 200; i++) //筛选所有可能的bound box
    {
        if(mode == 1 ? inf_out_t[6*i+4] > 0.00005 : inf_out_t[6*i+4] > 0.005)
        {
            if(head == NULL)
            {
                head = new bbox_info();
                tail = head;
            }
            else
            {
                p = new bbox_info();
                p -> last = tail;
                tail -> next = p;
                tail = tail -> next;
            }
            tail -> bbox_index = i;
            tail -> bbox_confidence = inf_out_t[6*i+4];
            counter++;
        }
    }

    if(head)
    {
        p = head;
        while(p) //计算所有可能bound box的四角坐标
        {
            if(mode)
            {
                p -> x1 = (inf_out_t[p -> bbox_index*6+0] - (inf_out_t[p -> bbox_index*6+2]/2));
                p -> x2 = (inf_out_t[p -> bbox_index*6+0] + (inf_out_t[p -> bbox_index*6+2]/2));
                p -> y1 = (inf_out_t[p -> bbox_index*6+1] - (inf_out_t[p -> bbox_index*6+3]/2));
                p -> y2 = (inf_out_t[p -> bbox_index*6+1] + (inf_out_t[p -> bbox_index*6+3]/2));
                p = p -> next;
            }
            else
            {
                p -> x1 = (inf_out_t[p -> bbox_index*6+0] - (inf_out_t[p -> bbox_index*6+2]/2)) * 2;
                p -> x2 = (inf_out_t[p -> bbox_index*6+0] + (inf_out_t[p -> bbox_index*6+2]/2)) * 2;
                p -> y1 = (inf_out_t[p -> bbox_index*6+1] - (inf_out_t[p -> bbox_index*6+3]/2)) * 2;
                p -> y2 = (inf_out_t[p -> bbox_index*6+1] + (inf_out_t[p -> bbox_index*6+3]/2)) * 2;
                p = p -> next;
            }
        }
    }
    else
    {
        return;
    }

    while(head)
    {
        float SI = 0.0;
        float SU = 0.0;
        float IoU = 0.0;

        p = head;
        q = head;
        while(p)
        {
            if(p -> bbox_confidence >= q -> bbox_confidence)
            {
                q = p;
                p = p -> next;
            }
            else
            {
                p = p -> next;
            }
        }

        p = head;
        while(p)
        {
            if(p != q)
            {
                if(p -> x2 < q -> x1||p -> x1 > q -> x2||p -> y2 < q -> y1||p -> y1 > q -> y2)
                {
                    SI = 0.0;
                }
                else
                {
                    float d_x = (p -> x2 < q -> x2 ? p -> x2 : q -> x2) - (p -> x1 > q -> x1 ? p -> x1 : q -> x1);
                    float d_y = (p -> y2 < q -> y2 ? p -> y2 : q -> y2) - (p -> y1 > q -> y1 ? p -> y1 : q -> y1);
                    SI = d_x * d_y;
                }
                SU =  (p -> x2 - p -> x1) * (p -> y2 - p -> y1) + (q -> x2 - q -> x1) * (q -> y2 - q -> y1) - SI;
                IoU = SI / SU;
                if(IoU > Threshold)
                {
                    bbox_info* temp1 = NULL;
                    if(p != head && p != tail)
                    {
                        p -> last -> next = p -> next;
                        p -> next -> last = p -> last;
                        temp1 = p;
                        p = p -> next;
                        delete temp1;
                    }
                    else if(p == head && p != tail)
                    {
                        head = head -> next;
                        head -> last = NULL;
                        temp1 = p;
                        p = p -> next;
                        delete temp1;
                    }
                    else if(p != head && p == tail)
                    {
                        tail = tail -> last;
                        tail -> next = NULL;
                        temp1 = p;
                        p = NULL;
                        delete temp1;
                    }
                    counter--;
                }
                else
                {
                    p = p -> next;
                }
            }
            else
            {
                p = p -> next;
            }
        }

        int x1i, x2i, y1i, y2i = 0;
        x1i = (int) q -> x1;
        x2i = (int) q -> x2;
        y1i = (int) q -> y1;
        y2i = (int) q -> y2;
        rectangle(bitmap, Point(x1i, y1i), Point(x2i, y2i), Scalar( 0, 255, 255 ), 2, 8);
        
        bbox_info* temp2 = NULL;
        if(q != head && q != tail)
        {
            q -> last -> next = q -> next;
            q -> next -> last = q -> last;
            temp2 = q;
            q = NULL;
            delete temp2;
        }
        else if(q == head && q != tail)
        {
            head = head -> next;
            head -> last = NULL;
            temp2 = q;
            q = NULL;
            delete temp2;
        }
        else if(q != head && q == tail)
        {
            tail = tail -> last;
            tail -> next = NULL;
            temp2 = q;
            q = NULL;
            delete temp2;
        }
        else if(q == head && q == tail)
        {
            head = NULL;
            tail = NULL;
            temp2 = q;
            q = NULL;
            delete temp2;
        }
        counter--;
    }
}

int process_normal(VideoCapture& capture, int w, int h, int counter)
{
    int index = 100;
    int mode = 0;

    char* filename = NULL;
    JpegEncoder EN;

    char str[20];
    double fps;
    clock_t begin, stop, time = 0;

    while (1)
    {
        begin = clock();               //starting time
        capture >> frame;
        if (frame.empty())
            break;

        resize(frame, frame, Size(w,h));

        filename = new char[30];

        if (!EN.readFromBMP(frame.cols, frame.rows, frame.data))
        {
            printf("input error\n");
            return 1;
        }

        sprintf(filename, "%s%d%s", "./picture/", counter, ".jpg");
        counter++;

        if (!EN.encodeToJPG(filename, index, &M, mode))
        {
            printf("save error\n");
            return 1;
        }

        JpegDecoder DE(filename);
        BitmapImage& img = DE.Decoder();

        delete[] filename;

        Mat bitmap;
        bitmap.create(img.Height, img.Width, CV_8UC3); // CV_8UC3 = 24位真彩色
        bitmap.data = img.Data;

        stop = clock();               //ending time
        time = stop - begin;  //time
        fps = 1.0 / (((double)time) / CLOCKS_PER_SEC);

        string FPSstring("FPS:");
        sprintf(str, "%.2f", fps);
        FPSstring += str;
        //在帧上显示"FPS:XXXX"
        putText(bitmap, FPSstring, Point(5, 20),
            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255));
        imshow("Bitmap", bitmap);
        if (waitKey(1) == 27)
        {
            return counter;
        }
    }
    return 0;
}

int process_recognition_picture(VideoCapture& capture, int counter)
{
    int index = 100;
    int mode = 0;

    char* filename = NULL;
    JpegEncoder EN;

    char str[20];
    double fps;
    clock_t begin, stop, time = 0;

//------------------搭建神经网络---------------------
    string modelFile = "./model/picture.mnn";             //mnn model path

    int numThread = 4;                             //number of thread

    // build network
    Interpreter* net = Interpreter::createFromFile(modelFile.c_str());

    // build config
    ScheduleConfig config;

    // set cpu thread used
    config.numThread = numThread;

    //set device type
    config.type = static_cast<MNNForwardType>(MNN_FORWARD_CPU);

    // set precision
    BackendConfig backendConfig;
    backendConfig.precision = static_cast<BackendConfig::PrecisionMode>(BackendConfig::Precision_High);

    // set power use
    backendConfig.power = static_cast<BackendConfig::PowerMode>(BackendConfig::Power_Normal);
    // set memory use
    backendConfig.memory = static_cast<BackendConfig::MemoryMode>(BackendConfig::Memory_Normal);

    config.backendConfig = &backendConfig;

    // build session use config
    Session* session = net->createSession(config);

    // get input and output node of network
    Tensor* modelInputTensor = net->getSessionInput(session, NULL);
    Tensor* modelOutputTensor = net->getSessionOutput(session, NULL);
//---------------------------------------------------------------

    while (1)
    {
        begin = clock();               //starting time
        capture >> frame;
        if (frame.empty())
            break;

        frame_crop = Rect(0, 80, 640, 320);
        frame = frame(frame_crop);

        filename = new char[30];

        if (!EN.readFromBMP(frame.cols, frame.rows, frame.data))
        {
            printf("input error\n");
            return 1;
        }

        sprintf(filename, "%s%d%s", "./picture/", counter, ".jpg");
        counter++;

        if (!EN.encodeToJPG(filename, index, &M, mode))
        {
            printf("save error\n");
            return 1;
        }

        JpegDecoder DE(filename);
        BitmapImage& img = DE.Decoder();

        delete[] filename;

        Mat bitmap;
        bitmap.create(img.Height, img.Width, CV_8UC3); // CV_8UC3 = 24位真彩色
        bitmap.data = img.Data;

        resize(bitmap, frame_clone, Size(320,160));         //new image size
        frame_clone.convertTo(frame_clone, CV_32F, 1 / 255.0);            //change data type to float32, and adjust range

        //set input and output tensor
        Tensor* inputTensor = Tensor::create<float>({1, 160, 320, 3}, NULL, Tensor::TENSORFLOW);
        Tensor* outputTensor = Tensor::create<float>({1, 1200, 6}, NULL, Tensor::CAFFE);
        memcpy(inputTensor->host<float>(), frame_clone.data, inputTensor->size());
        // inference
        modelInputTensor->copyFromHostTensor(inputTensor);                   //inference
        net->runSession(session);
        modelOutputTensor->copyToHostTensor(outputTensor);

        // post-process
        float inf_out[6][1200] = {0};
        memcpy(inf_out, outputTensor->host<float>(), outputTensor->size());

        float inf_out_t[1200] = {0};
        for(int i = 0; i < 1200; i++)
        {
            for(int j = 0; j < 6; j++)
            {
                inf_out_t[i] += inf_out[j][i];
            }
            inf_out_t[i] = inf_out_t[i]/6;
        }

        NMS(inf_out_t, bitmap, mode);

        stop = clock();               //ending time
        time = stop - begin;  //time
        fps = 1.0 / (((double)time) / CLOCKS_PER_SEC);

        string FPSstring("FPS:");
        sprintf(str, "%.2f", fps);
        FPSstring += str;
        //在帧上显示"FPS:XXXX"
        putText(bitmap, FPSstring, Point(5, 20),
            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255));
        imshow("Bitmap", bitmap);
        if (waitKey(1) == 27)
        {
            return counter;
        }
    }
    return 0;
}

int process_recognition_frequency(VideoCapture& capture, int counter)
{
    int index = 100;
    int mode = 1;

    char* filename = NULL;
    JpegEncoder EN;

    char str[20];
    double fps;
    clock_t begin, stop, time = 0;

//------------------搭建神经网络---------------------
    string modelFile = "./model/frequency.mnn";             //mnn model path

    int numThread = 4;                             //number of thread

    // build network
    Interpreter* net = Interpreter::createFromFile(modelFile.c_str());

    // build config
    ScheduleConfig config;

    // set cpu thread used
    config.numThread = numThread;

    //set device type
    config.type = static_cast<MNNForwardType>(MNN_FORWARD_CPU);

    // set precision
    BackendConfig backendConfig;
    backendConfig.precision = static_cast<BackendConfig::PrecisionMode>(BackendConfig::Precision_High);

    // set power use
    backendConfig.power = static_cast<BackendConfig::PowerMode>(BackendConfig::Power_Normal);
    // set memory use
    backendConfig.memory = static_cast<BackendConfig::MemoryMode>(BackendConfig::Memory_Normal);

    config.backendConfig = &backendConfig;

    // build session use config
    Session* session = net->createSession(config);

    // get input and output node of network
    Tensor* modelInputTensor = net->getSessionInput(session, NULL);
    Tensor* modelOutputTensor = net->getSessionOutput(session, NULL);

    while (1)
    {
        begin = clock();               //starting time
        capture >> frame;
        if (frame.empty())
            break;

        resize(frame, frame, Size(640,480));
        frame_crop = Rect(0, 80, 640, 320);//640,320
        frame = frame(frame_crop);

        filename = new char[30];

        if (!EN.readFromBMP(frame.cols, frame.rows, frame.data))
        {
            printf("input error\n");
            return 1;
        }

        sprintf(filename, "%s%d%s", "./picture/", counter, ".jpg");
        counter++;

        if (!EN.encodeToJPG(filename, index, &M, mode))
        {
            printf("save error\n");
            return 1;
        }

        JpegDecoder DE(filename);
        BitmapImage& img = DE.Decoder();

        delete[] filename;

        Mat bitmap;
        bitmap.create(img.Height, img.Width, CV_8UC3); // CV_8UC3 = 24位真彩色
        bitmap.data = img.Data;

        for(int c = 0; c < 32; c++)
        {
            for(int y = 0; y < 40; y++)
            {
                for(int x = 0; x < 80; x++)
                {
                    M.at<Vec32f>(y, x)[c] = (M.at<Vec32f>(y, x)[c] - sel_chan_mean[c]) / sel_chan_var_std[c];
                }
            }
        }
        //set input and output tensor
        Tensor* inputTensor = Tensor::create<float>({1, 40, 80, 32}, NULL, Tensor::TENSORFLOW);
        Tensor* outputTensor = Tensor::create<float>({1, 1200, 6}, NULL, Tensor::CAFFE);
        memcpy(inputTensor->host<float>(), M.data, inputTensor->size());
        // inference
        modelInputTensor->copyFromHostTensor(inputTensor);                   //inference
        net->runSession(session);
        modelOutputTensor->copyToHostTensor(outputTensor);

        // post-process
        float inf_out[6][1200] = {0};
        memcpy(inf_out, outputTensor->host<float>(), outputTensor->size());

        float inf_out_t[1200] = {0};
        for(int i = 0; i < 1200; i++)
        {
            for(int j = 0; j < 6; j++)
            {
                inf_out_t[i] += inf_out[j][i];
            }
            inf_out_t[i] = inf_out_t[i]/6;
        }

        NMS(inf_out_t, bitmap, mode);

        stop = clock();               //ending time
        time = stop - begin;  //time
        fps = 1.0 / (((double)time) / CLOCKS_PER_SEC);

        string FPSstring("FPS:");
        sprintf(str, "%.2f", fps);
        FPSstring += str;
        //在帧上显示"FPS:XXXX"
        putText(bitmap, FPSstring, Point(5, 20),
            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255));
        imshow("Bitmap", bitmap);
        if (waitKey(1) == 27)
        {
            return counter;
        }
    }
    return 0;
}