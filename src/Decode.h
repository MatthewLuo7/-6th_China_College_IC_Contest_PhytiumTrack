/*
 * \file Decode.h
 * \解码部分头文件
 */

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>
#include <string>
#include <iostream>
#include <arm_neon.h>

#include "Tree.h"

#define frbuff_size 32768

class BitmapImage
{
public:
    BitmapImage() : Data(NULL), Height(0), Width(0) {}
    ~BitmapImage()
    {
        if (Data != NULL)
            delete[]Data;
    }

    void CreateImage()
    {
        Data = new uint8_t[Height * Width * 3];
    }

    uint8_t* Data;  // image data
    int     Height; // rows of image
    int     Width;  // cols of image
};

static int const UnZigZagTable[64] =
{
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 41, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

/* @brief 离散余弦变换系数矩阵
 */
static const double MtxDCT[8][8] =
{
    {0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536},
    {0.4904,    0.4157,    0.2778,    0.0975,   -0.0975,   -0.2778,   -0.4157,   -0.4904},
    {0.4619,    0.1913,   -0.1913,   -0.4619,   -0.4619,   -0.1913,    0.1913,    0.4619},
    {0.4157,   -0.0975,   -0.4904,   -0.2778,    0.2778,    0.4904,    0.0975,   -0.4157},
    {0.3536,   -0.3536,   -0.3536,    0.3536,    0.3536,   -0.3536,   -0.3536,    0.3536},
    {0.2778,   -0.4904,    0.0975,    0.4157,   -0.4157,   -0.0975,    0.4904,   -0.2778},
    {0.1913,   -0.4619,    0.4619,   -0.1913,   -0.1913,    0.4619,   -0.4619,    0.1913},
    {0.0975,   -0.2778,    0.4157,   -0.4904,    0.4904,   -0.4157,    0.2778,   -0.0975}
};

/* @brief 反离散余弦变换系数矩阵
 */
static const double MtxIDCT[8][8] =
{
    {0.3536,    0.4904,    0.4619,    0.4157,    0.3536,    0.2778,    0.1913,    0.0975},
    {0.3536,    0.4157,    0.1913,   -0.0975,   -0.3536,   -0.4904,   -0.4619,   -0.2778},
    {0.3536,    0.2778,   -0.1913,   -0.4904,   -0.3536,    0.0975,    0.4619,    0.4157},
    {0.3536,    0.0975,   -0.4619,   -0.2778,    0.3536,    0.4157,   -0.1913,   -0.4904},
    {0.3536,   -0.0975,   -0.4619,    0.2778,    0.3536,   -0.4157,   -0.1913,    0.4904},
    {0.3536,   -0.2778,   -0.1913,    0.4904,   -0.3536,   -0.0975,    0.4619,   -0.4157},
    {0.3536,   -0.4157,    0.1913,    0.0975,   -0.3536,    0.4904,   -0.4619,    0.2778},
    {0.3536,   -0.4904,    0.4619,   -0.4157,    0.3536,   -0.2778,    0.1913,   -0.0975}
}; 

/* Matrix */
class Mtx
{
public:
    int* operator[] (int row)
    {
        return data[row];
    }
private:
    int data[8][8];
};


class Pixel
{
public:
    int R, G, B;
};

/*  Y : Cr : Cb  =  4 : 1 : 1 */
class MCU
{
public:
    Mtx    yMtx[4];
    Mtx    crMtx;
    Mtx    cbMtx;
};

class HuffmanTable
{
public:
    std::map<std::string, uint8_t>& operator[] (int index)
    {
        switch (index)
        {
        case 0:  return DC[0]; break; /* 00 */
        case 1:  return DC[1]; break; /* 01 */
        case 16: return AC[0]; break; /* 10 */
        case 17: return AC[1]; break; /* 11 */
        default: return DC[0]; break;
        }
    }
private:
    std::map<std::string, uint8_t> DC[2];
    std::map<std::string, uint8_t> AC[2];
};


struct fLUT
{
    unsigned char symbol;
    unsigned char bitlen;
    fLUT(): symbol(0x00), bitlen(0x00) {};
};

class JpegDecoder
{
    private:
    BitmapImage  img; // image data
    FILE* fp;
    uint8_t      quantY[64]; // Y 量化表
    uint8_t      quantC[64]; // Cr, Cb 共用量化表
    HuffmanTable huffman;    // 哈夫曼表
    MCU          mcu;        // 最小编码（解码）单元
    int          dcY;        // Y 分量的 DC 差分值
    int          dcCr;       // Cr 分量的 DC 差分值
    int          dcCb;       // Cb 分量的 DC 差分值

    Pixel        rgbBuf[16][16]; // 解码后的RGB值
    int          idxOfBlock;  // index of block number
    int          xNumberOfBlock;
    int          yNumberOfBlock;

    bool        endOfDecoder;
    uint8_t     currentVal; // 当前解码数据
    // uint8_t      readCount;  // 已读取 bit 数, >=8 时从文件读取下一个字节数据到 currentVal
    int     readCount;
    Tree TDC0,TDC1,TAC0,TAC1;
    uint8_t     *frbuff;//read buffer
    int actual_size, current_pos;

    fLUT FLDC0[256], FLDC1[256], FLAC0[256], FLAC1[256];

    int lutbuff, lutbnum;

    public:

    JpegDecoder(char* fileName);
    ~JpegDecoder();
    BitmapImage& Decoder();

protected:
    /* @brief Decoder a matrix of mcu
       @params:
            block: 返回值
            table: huffman table type ( 00, 01, 10, 11 )
            quant: 量化表
    */
    void DecoderMtx(Mtx& block, int table, uint8_t* quant, int& dc);

    /*  计算下一个MCU */
    void DecoderNextMCU();

    /* 读取哈夫曼表 */
    void DecoderHuffman();

    void getLUT(TreeNode* tn, fLUT fastlut[], int dep, unsigned char code);

    /* 解码 一个哈夫曼表 */
    void DecoderTable();

    /* 读取量化表 */
    void DecoderQuant();

    /* 读取图像 宽高 */
    void DecoderSize();

    /* 计算矩阵乘积 */
    Mtx MtxMulI2D(Mtx& left, const double right[8][8]);

    /* 计算矩阵乘积 */
    Mtx MtxMulD2I(const double left[8][8], Mtx& right);

    void UnZigZag(Mtx& block);

    /* 获取查哈夫曼表后得到真实数据的有效长度， 读取指定的位数 */
    int GetRealValue(int length);

    void DCT(Mtx& block)
    {
        block = MtxMulD2I(MtxDCT, block);
        block = MtxMulI2D(block, MtxIDCT);
    }

    /* 反离散余弦变换 */
    void IDCT(Mtx& block)
    {
        IAAN_2D(block);
    }

    float matA[8][8] =
    {
    {0.353553,0.353553,0.353553,0.353553,0.353553,0.353553,0.353553,0.353553},
    {0.490402,0.415818,0.277992,0.097887,-0.097106,-0.277330,-0.415375,-0.490246},
    {0.461978,0.191618,-0.190882,-0.461673,-0.462282,-0.192353,0.190146,0.461366},
    {0.415818,-0.097106,-0.490246,-0.278654,0.276667,0.490710,0.099448,-0.414486},
    {0.353694,-0.353131,-0.354257,0.352567,0.354818,-0.352002,-0.355379,0.351436},
    {0.277992,-0.490246,0.096325,0.416700,-0.414486,-0.100228,0.491014,-0.274674},
    {0.191618,-0.462282,0.461366,-0.189409,-0.193822,0.463187,-0.460440,0.187196},
    {0.097887,-0.278654,0.416700,-0.490863,0.489771,-0.413593,0.274008,-0.092414}
    };

    float matAT[8][8] =
    {
    {0.353553,0.490402,0.461978,0.415818,0.353694,0.277992,0.191618,0.097887},
    {0.353553,0.415818,0.191618,-0.097106,-0.353131,-0.490246,-0.462282,-0.278654},
    {0.353553,0.277992,-0.190882,-0.490246,-0.354257,0.096325,0.461366,0.416700},
    {0.353553,0.097887,-0.461673,-0.278654,0.352567,0.416700,-0.189409,-0.490863},
    {0.353553,-0.097106,-0.462282,0.276667,0.354818,-0.414486,-0.193822,0.489771},
    {0.353553,-0.277330,-0.192353,0.490710,-0.352002,-0.100228,0.463187,-0.413593},
    {0.353553,-0.415375,0.190146,0.099448,-0.355379,0.491014,-0.460440,0.274008},
    {0.353553,-0.490246,0.461366,-0.414486,0.351436,-0.274674,0.187196,-0.092414}
    };

    void mul_88(float(*matx)[8], Mtx& maty, float(*matres)[8]);
    void mul_88_(float(*matx)[8], float(*maty)[8], Mtx& matres);

    void IAAN_1D(float* dataptr);
    void IAAN_2D(Mtx& matdata);

    /* 反采样 */
    Mtx InveseSample(Mtx& block, int number);

    /* 颜色空间转换 */
    void ConvertClrSpace(Mtx& Y, Mtx& Cr, Mtx& Cb, Pixel out[8][8]);

    /* 将 MCU 单元 YCrCb 颜色转换为 RGB 颜色 */
    void Convert();

    void WriteToRGBBuffer(Pixel buf[8][8], int blockIndex);

    /* 重置 DC 值到 0 */
    void ResetDC()
    {
        dcY = dcCr = dcCb = 0x0;
    }

    void WirteBlock(Pixel block[16][16]);

    /* 定位到数据流的第一个字节 */
    void ToStartOfData()
    {
        int offset = 607; // 文件头的长度
        fseek(fp, offset, SEEK_SET);  // 头文件头开始
        fread(&currentVal, 1, 1, fp); // 读取第一个数据
    }

    /* @brief 获取下一个 Key 的 value
        @params:
                table: type of huffman ( 00, 01, 10, 11 )
    */
    unsigned char FindKeyValue(int table);

    int ReadFromFile();

    int NextByte();
};