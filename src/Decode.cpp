/*
 * \file Decode.cpp
 * \解码部分主体
 */

#include "Decode.h"

using namespace std;

JpegDecoder::JpegDecoder(char* fileName): dcY(0), dcCr(0), dcCb(0), idxOfBlock(0), endOfDecoder(false), readCount(7)
{
    fp = fopen(fileName, "rb+");
    if (fp == NULL) printf("no input");
    
    actual_size = 0;
    current_pos = 0;

    frbuff = new uint8_t[frbuff_size];

    lutbuff = 0;
    lutbnum = 0;
}

JpegDecoder::~JpegDecoder()
{
    delete frbuff;
    frbuff = NULL;
    fclose(fp);
}

/* @brief 解码
 */
BitmapImage& JpegDecoder::Decoder()
{
    /* decoder quant table */
    DecoderQuant();

    /* decoder width and height of image */
    DecoderSize();

    /* decoder huffman table of DC and AC */
    DecoderHuffman();

    /* decoder data */
    ToStartOfData();

    /* decoder MCU */

    int totalBlock = xNumberOfBlock * yNumberOfBlock;
    while (!endOfDecoder && (idxOfBlock < totalBlock))
    {
        DecoderNextMCU();

        Convert();

        WirteBlock(rgbBuf);
    }
    /* end of decoder */
    return img;
}

/* @brief 解码一个 8 x 8 的矩阵块
 */
void JpegDecoder::DecoderMtx(Mtx& block, int table, uint8_t* quant, int& dc)
{
    if (endOfDecoder) return; // 解码终止

    // reset matrix
    for (int i = 0; i < 64; i++) block[i / 8][i % 8] = 0x0;

    // decoder DC of matrix
    unsigned int length = FindKeyValue(table);
    int value = GetRealValue(length);
    dc += value; // DC
    block[0][0] = dc;

    // decoder AC of matrix, table = table + 16 => table = 0x00 (DC-0)-> table = 0x10 (AC-0)

    for (int i = 1; i < 64; i++)
    {
        length = FindKeyValue(table + 16);

        if (length == 0x0) break; // 结束条件
        value = GetRealValue(length & 0xf); // 右边 4位，实际值长度
        i += (length >> 4);          // 左边 4位，行程长度
        block[i / 8][i % 8] = value; // AC
    }

    // 反量化
    for (int i = 0; i < 64; i++) block[i / 8][i % 8] *= quant[i];

    // 反 Zig-Zag 编码
    UnZigZag(block);

    // 反离散余弦变换
    IDCT(block);
}

/* @brief 解码一个 MCU
 */
void JpegDecoder::DecoderNextMCU()
{
    /* Y 分量， 直流 0号表 */
    DecoderMtx(mcu.yMtx[0], 0, quantY, dcY);
    DecoderMtx(mcu.yMtx[1], 0, quantY, dcY);
    DecoderMtx(mcu.yMtx[2], 0, quantY, dcY);
    DecoderMtx(mcu.yMtx[3], 0, quantY, dcY);

    /* Cb, Cr 分量， 直流 1号表 */
    DecoderMtx(mcu.cbMtx, 1, quantC, dcCr);
    DecoderMtx(mcu.crMtx, 1, quantC, dcCb);
}

/* 读取哈夫曼表 */
void JpegDecoder::DecoderHuffman()
{
    int offset = 177;
    fseek(fp, offset, SEEK_SET);
    DecoderTable();         /* DC-0 */

    offset = 0;
    fseek(fp, offset, SEEK_CUR);
    DecoderTable();         /* AC-0 */

    fseek(fp, offset, SEEK_CUR);
    DecoderTable();         /* DC-1 */

    fseek(fp, offset, SEEK_CUR);
    DecoderTable();         /* AC-1 */

    for(int i=0;i<256;i++)
    {
        FLDC0[i].symbol = 0xff;
        FLDC0[i].bitlen = 0;
        FLDC1[i].symbol = 0xff;
        FLDC1[i].bitlen = 0;
        FLAC0[i].symbol = 0xff;
        FLAC0[i].bitlen = 0;
        FLAC1[i].symbol = 0xff;
        FLAC1[i].bitlen = 0;
    }
    getLUT(TDC0.getroot(),FLDC0,0,0);
    getLUT(TDC1.getroot(),FLDC1,0,0);
    getLUT(TAC0.getroot(),FLAC0,0,0);
    getLUT(TAC1.getroot(),FLAC1,0,0);
}

void JpegDecoder::getLUT(TreeNode* tn, fLUT fastlut[], int dep, unsigned char code)
{
    if((dep <= 8)&&(tn != NULL))
    {
        getLUT(tn->Lchild, fastlut, dep+1, (code<<1));
        getLUT(tn->Rchild, fastlut, dep+1, (code<<1)+1);
        if((tn->Lchild == NULL)&&(tn->Rchild == NULL))
        {
            code = code<<(8-dep);
            int makeup = 1<<(8-dep);
            for(int i=0;i<makeup;i++)
            {
                int temp = code+i;
                fastlut[temp].symbol = tn->value;
                fastlut[temp].bitlen = dep;
            }
        }
    }
}

/* @brief 解码哈夫曼表
 */
void JpegDecoder::DecoderTable()
{
    int val = 0; // type of table ( DC(0,1) or AC(0,1) )
    uint8_t buf[16];

    fread(&val, 1, 1, fp); // get type of table
    int treeNO = val;
    fread(buf, 1, 16, fp); // get key value
    map<string, uint8_t>& tb = huffman[val];

    string keyStr = "";
    for (int i = 0; i < 16; i++) // length of key (i.e. i = 2 means key = 000 , 001 , 010 , 011 or ...)
    {
        int cnt = buf[i]; // number of key, which length is (i+1)

        /* alignment */
        for (int k = keyStr.length(); k <= i; k++)
        {
            keyStr += "0";
        }

        while (cnt > 0)
        {
            /* value of key */
            fread(&val, 1, 1, fp); // read value
            switch(treeNO)
            {
                case 0 :TDC0.AddNode(val,i+1);break;
                case 1 :TDC1.AddNode(val,i+1);break;
                case 16:TAC0.AddNode(val,i+1);break;
                case 17:TAC1.AddNode(val,i+1);break;
            }
            tb.insert(pair<string, uint8_t>(keyStr, val));

            /* increment */
            int carry = 1; //进位
            for (int k = keyStr.length() - 1; k >= 0; k--)
            {
                int tmpVal = (keyStr[k] + carry - '0'); //计算进位
                carry = tmpVal / 2;
                keyStr[k] = tmpVal % 2 + '0'; //计算后当前位结果
            }

            cnt = cnt - 1;
        }
    }
}

void JpegDecoder::DecoderQuant()
{
    /* read Quant Table of Y */
    int offset = 25;
    fseek(fp, offset, SEEK_SET);
    fread(quantY, 1, 64, fp);

    /* read Quant Table of C */
    offset = 1;
    fseek(fp, offset, SEEK_CUR);
    fread(quantC, 1, 64, fp);
}

void JpegDecoder::DecoderSize()
{
    uint8_t buf[2];
    /* read height */
    int offset = 159;
    fseek(fp, offset, SEEK_SET);
    fread(buf, 1, 2, fp);
    img.Height = (buf[0] << 8) + buf[1];

    /* read width */
    fread(buf, 1, 2, fp);
    img.Width = (buf[0] << 8) + buf[1];

    /* create image */
    img.CreateImage();

    /* compute number of MCU block */
    xNumberOfBlock = (img.Height + 15) / 16;
    yNumberOfBlock = (img.Width + 15) / 16;
}

Mtx JpegDecoder::MtxMulI2D(Mtx& left, const double right[8][8])
{
    Mtx dctBuf;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            double tempVal = 0.0;
            for (int k = 0; k < 8; k++)
            {
                tempVal += left[i][k] * right[k][j];
            }
            dctBuf[i][j] = round(tempVal);
        }
    }
    return dctBuf;
}

Mtx JpegDecoder::MtxMulD2I(const double left[8][8], Mtx& right)
{
    Mtx dctBuf;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            double tempVal = 0.0;
            for (int k = 0; k < 8; k++)
            {
                tempVal += left[i][k] * right[k][j];
            }
            dctBuf[i][j] = round(tempVal);
        }
    }
    return dctBuf;
}

void JpegDecoder::UnZigZag(Mtx& block)
{
    int tempBuf[64];
    for (int i = 0; i < 64; i++) tempBuf[i] = block[i / 8][i % 8];

    for (int i = 0; i < 64; i++) block[i / 8][i % 8] = tempBuf[UnZigZagTable[i]];
}

/* 获取标准哈夫曼编码的真实值 */
int JpegDecoder::GetRealValue(int length)
{
    int retVal = 0;
    if(lutbnum <= length)
    {
        for(int i = lutbnum; i < length; i = i+8)
        {
            lutbuff = (lutbuff << 8) + NextByte();
            lutbnum += 8;
        }
        retVal = (lutbuff >> (lutbnum - length));
        lutbnum -= length;
        lutbuff = (lutbuff & (~(-1 << lutbnum)));
    }
    else
    {
        retVal = (lutbuff & (~(-1 << lutbnum)));
        retVal = retVal >> (lutbnum - length);
        lutbnum -= length;
        lutbuff = (lutbuff & (~(-1 << lutbnum)));
    }

    return ((((retVal>>(length-1)) & 0x1) == 0x1) ? retVal : (retVal + 1 - (0x1<<length)));
}

void JpegDecoder::mul_88(float(*matx)[8], Mtx& maty, float(*matres)[8])
{
    int i, j, k = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            matres[i][j] = 0.f;
            for (k = 0; k < 8; k++)
            {
                matres[i][j] += matx[i][k] * maty[k][j];
            }
        }
    }
}

void JpegDecoder::mul_88_(float(*matx)[8], float(*maty)[8], Mtx& matres)
{
    int i, j, k = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            matres[i][j] = 0.f;
            for (k = 0; k < 8; k++)
            {
                matres[i][j] += matx[i][k] * maty[k][j];
            }
        }
    }
}

void JpegDecoder::IAAN_1D(float* dataptr)
{
    float AANS[8] = { 0.353558, 0.490402, 0.453293, 0.415818, 0.353694, 0.277992, 0.191618, 0.097887 };
    float32x4_t tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp10, tmp11, tmp12, tmp13;
    float32x4_t z1, z2, z3, z4, z11, z13;

    float32x4_t DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7;

    float32x4_t aans, a, b, c, d, n;

    DATA0 = vld1q_f32(dataptr);
    DATA1 = vld1q_f32(dataptr + 4);
    DATA2 = vld1q_f32(dataptr + 8);
    DATA3 = vld1q_f32(dataptr + 12);
    DATA4 = vld1q_f32(dataptr + 16);
    DATA5 = vld1q_f32(dataptr + 20);
    DATA6 = vld1q_f32(dataptr + 24);
    DATA7 = vld1q_f32(dataptr + 28);

    aans = vmovq_n_f32(AANS[0]);
    DATA0 = vmulq_f32(aans, DATA0);
    aans = vmovq_n_f32(AANS[1]);
    DATA1 = vmulq_f32(aans, DATA1);
    aans = vmovq_n_f32(AANS[2]);
    DATA2 = vmulq_f32(aans, DATA2);
    aans = vmovq_n_f32(AANS[3]);
    DATA3 = vmulq_f32(aans, DATA3);
    aans = vmovq_n_f32(AANS[4]);
    DATA4 = vmulq_f32(aans, DATA4);
    aans = vmovq_n_f32(AANS[5]);
    DATA5 = vmulq_f32(aans, DATA5);
    aans = vmovq_n_f32(AANS[6]);
    DATA6 = vmulq_f32(aans, DATA6);
    aans = vmovq_n_f32(AANS[7]);
    DATA7 = vmulq_f32(aans, DATA7);

    z13 = vaddq_f32(DATA5, DATA3);
    z2 = vsubq_f32(DATA5, DATA3);
    z11 = vaddq_f32(DATA1, DATA7);
    z4 = vsubq_f32(DATA1, DATA7);

    tmp7 = vaddq_f32(z11, z13);
    z3 = vsubq_f32(z11, z13);

    n = vmovq_n_f32(1.414213);
    tmp11 = vmulq_f32(z3, n);
    a = vmovq_n_f32(2.414215);
    tmp12 = vmulq_f32(a, z4);
    b = vmovq_n_f32(0.382683);
    tmp12 = vsubq_f32(tmp12, z2);
    tmp12 = vmulq_f32(tmp12, b);
    d = vmovq_n_f32(2);
    tmp12 = vmulq_f32(tmp12, d);
    tmp10 = vmulq_f32(b, tmp12);
    c = vmovq_n_f32(1.082393);
    z2 = vmulq_f32(z2, d);
    tmp10 = vaddq_f32(z2, tmp10);
    tmp10 = vmulq_f32(c, tmp10);

    tmp6 = vsubq_f32(tmp12, tmp7);
    tmp5 = vsubq_f32(tmp11, tmp6);
    tmp4 = vsubq_f32(tmp10, tmp5);

    tmp13 = vaddq_f32(DATA2, DATA6);
    z1 = vsubq_f32(DATA2, DATA6);
    tmp12 = vmulq_f32(z1, n);
    tmp12 = vsubq_f32(tmp12, tmp13);

    tmp10 = vaddq_f32(DATA0, DATA4);
    tmp11 = vsubq_f32(DATA0, DATA4);

    tmp1 = vaddq_f32(tmp11, tmp12);
    tmp2 = vsubq_f32(tmp11, tmp12);
    tmp0 = vaddq_f32(tmp10, tmp13);
    tmp3 = vsubq_f32(tmp10, tmp13);
    
    DATA0 = vaddq_f32(tmp0, tmp7);
    DATA7 = vsubq_f32(tmp0, tmp7);
    DATA1 = vaddq_f32(tmp1, tmp6);
    DATA6 = vsubq_f32(tmp1, tmp6);
    DATA2 = vaddq_f32(tmp2, tmp5);
    DATA5 = vsubq_f32(tmp2, tmp5);
    DATA3 = vaddq_f32(tmp3, tmp4);
    DATA4 = vsubq_f32(tmp3, tmp4);

    vst1q_f32(dataptr, DATA0);
    vst1q_f32(dataptr + 4, DATA1);
    vst1q_f32(dataptr + 8, DATA2);
    vst1q_f32(dataptr + 12, DATA3);
    vst1q_f32(dataptr + 16, DATA4);
    vst1q_f32(dataptr + 20, DATA5);
    vst1q_f32(dataptr + 24, DATA6);
    vst1q_f32(dataptr + 28, DATA7);
}

void JpegDecoder::IAAN_2D(Mtx& matdata)
{
    float matemp[64] = { 0 };
    float arraytemp[32] = { 0 };
    int i, j = 0;
    for (i = 0; i < 8; i += 4)
    {
        for (j = 0; j < 8; j++)
        {
            arraytemp[j * 4] = matdata[i][j];
            arraytemp[j * 4 + 1] = matdata[i + 1][j];
            arraytemp[j * 4 + 2] = matdata[i + 2][j];
            arraytemp[j * 4 + 3] = matdata[i + 3][j];
        }
        IAAN_1D(arraytemp);
        for (j = 0; j < 8; j++)
        {
            matemp[i * 8 + j] = arraytemp[j * 4];
            matemp[(i + 1) * 8 + j] = arraytemp[j * 4 + 1];
            matemp[(i + 2) * 8 + j] = arraytemp[j * 4 + 2];
            matemp[(i + 3) * 8 + j] = arraytemp[j * 4 + 3];
        }
    }
    for (i = 0; i < 8; i += 4)
    {
        for (j = 0; j < 8; j++)
        {
            arraytemp[j * 4] = matemp[j * 8 + i];
            arraytemp[j * 4 + 1] = matemp[j * 8 + i + 1];
            arraytemp[j * 4 + 2] = matemp[j * 8 + i + 2];
            arraytemp[j * 4 + 3] = matemp[j * 8 + i + 3];
        }
        IAAN_1D(arraytemp);
        for (j = 0; j < 8; j++)
        {
            matdata[j][i] = arraytemp[j * 4];
            matdata[j][i + 1] = arraytemp[j * 4 + 1];
            matdata[j][i + 2] = arraytemp[j * 4 + 2];
            matdata[j][i + 3] = arraytemp[j * 4 + 3];
        }
    }
}

Mtx JpegDecoder::InveseSample(Mtx& block, int number)
{
    Mtx ret;

    int x = (number / 2) * 4;
    int y = (number % 2) * 4;
    for (int i = 0; i < 8; i += 2)
    {
        for (int j = 0; j < 8; j += 2)
        {
            ret[i][j] = ret[i][j + 1] = ret[i + 1][j] = ret[i + 1][j + 1] = block[x + i / 2][y + j / 2];
        }
    }

    return ret;
}

/* @brief 颜色空间转换 YCbCr -> RGB
 */
void JpegDecoder::ConvertClrSpace(Mtx& Y, Mtx& Cb, Mtx& Cr, Pixel out[8][8])
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            out[i][j].R = Y[i][j] + 1.402 * Cr[i][j] + 128;
            out[i][j].G = Y[i][j] - 0.34414 * Cb[i][j] - 0.71414 * Cr[i][j] + 128;
            out[i][j].B = Y[i][j] + 1.772 * Cb[i][j] + 128;

            /* 截断 */
            if (out[i][j].R > 255) out[i][j].R = 255;
            if (out[i][j].G > 255) out[i][j].G = 255;
            if (out[i][j].B > 255) out[i][j].B = 255;

            /* 截断 */
            if (out[i][j].R < 0) out[i][j].R = 0;
            if (out[i][j].G < 0) out[i][j].G = 0;
            if (out[i][j].B < 0) out[i][j].B = 0;
        }
    }
}

/* @brief 调用 ConvertClrSpace(), 将 MCU 中的 YCbCr 颜色转换为 RGB 颜色
 */
void JpegDecoder::Convert()
{
    Mtx cb;
    Mtx cr;
    Pixel out[8][8];

    cb = InveseSample(mcu.cbMtx, 0);
    cr = InveseSample(mcu.crMtx, 0);
    ConvertClrSpace(mcu.yMtx[0], cb, cr, out);
    WriteToRGBBuffer(out, 0);

    cb = InveseSample(mcu.cbMtx, 1);
    cr = InveseSample(mcu.crMtx, 1);
    ConvertClrSpace(mcu.yMtx[1], cb, cr, out);
    WriteToRGBBuffer(out, 1);

    cb = InveseSample(mcu.cbMtx, 2);
    cr = InveseSample(mcu.crMtx, 2);
    ConvertClrSpace(mcu.yMtx[2], cb, cr, out);
    WriteToRGBBuffer(out, 2);

    cb = InveseSample(mcu.cbMtx, 3);
    cr = InveseSample(mcu.crMtx, 3);
    ConvertClrSpace(mcu.yMtx[3], cb, cr, out);
    WriteToRGBBuffer(out, 3);
}

/* @brief 把解码出来的RGB数据写入到RGB缓冲区中
    @buf: 像素数据
    @blockIndex: 块序号， 取值范围: 00, 01, 10, 11 (二进制) -> 0，1，2，3 (十进制)
 */
void JpegDecoder::WriteToRGBBuffer(Pixel buf[8][8], int blockIndex)
{
    int xOffset = 8 * (blockIndex & 0x02) >> 1; // binary: blockIndex & 10 (i.e. if blockIndex =  01 => blockIndex & 0x02 == 01 & 10 => xOffset = 0 * 8 = 0 )
    int yOffset = 8 * (blockIndex & 0x01);      // binary: blockIndex & 01 (i.e. if blockIndex =  01 => blockIndex & 0x01 == 01 & 01 => yOffset = 1 * 8 = 8 )
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            rgbBuf[xOffset + i][yOffset + j].R = buf[i][j].R;
            rgbBuf[xOffset + i][yOffset + j].G = buf[i][j].G;
            rgbBuf[xOffset + i][yOffset + j].B = buf[i][j].B;
        }
    }
}

void JpegDecoder::WirteBlock(Pixel block[16][16])
{
    int x = (idxOfBlock / yNumberOfBlock) * 16; //
    int y = (idxOfBlock % yNumberOfBlock) * 16; //
    idxOfBlock++;

    for (int i = 0; i < 16; i++)
    {
        if ((x + i) >= img.Height) break;
        for (int j = 0; j < 16; j++)
        {
            if (y + j >= img.Width) continue;
            int offset = ((x + i) * img.Width + (y + j)) * 3;
            img.Data[offset + 0] = block[i][j].B;
            img.Data[offset + 1] = block[i][j].G;
            img.Data[offset + 2] = block[i][j].R;
        }
    }
}

/* @brief 查找哈夫曼表，获取下一个有效值
 */
unsigned char JpegDecoder::FindKeyValue(int table)
{
    TreeNode* p = NULL;
    switch(table)
    {
    case 0 : p = TDC0.getroot(); break;
    case 1 : p = TDC1.getroot(); break;
    case 16: p = TAC0.getroot(); break;
    case 17: p = TAC1.getroot(); break;
    default: printf("default!!!\n"); break;
    }

    unsigned char res = 0x00;

    if(lutbnum < 8)
    {
        lutbuff = (lutbuff << 8) + NextByte();
        lutbnum += 8;
    }

    int lutemp = 0;
    lutemp = (lutbuff >> (lutbnum - 8));

    fLUT flres;
    switch(table)
    {
        case 0 : flres = FLDC0[lutemp];break;
        case 1 : flres = FLDC1[lutemp];break;
        case 16: flres = FLAC0[lutemp];break;
        case 17: flres = FLAC1[lutemp];break;
        default: printf("default!!!\n"); break;
    }

    if(flres.bitlen > 0)
    {
        res = flres.symbol;
        lutbnum -= flres.bitlen;
        lutbuff = (lutbuff & (~(-1 << lutbnum)));

        return res;
    }

    while(1)
    {
        int j;
        if(lutbnum <= 0)
        {
            lutbuff = NextByte();
            lutbnum = 8;
        }
        j = (lutbuff & (0x1 << (lutbnum - 1))) ? 1:0;
        lutbnum--;
        lutbuff = (lutbuff & (~(-1 << lutbnum)));
        if(j == 0)
        {
            p = p->Lchild;
        }
        else
        {
            p = p->Rchild;
        }
        if(p == NULL)
        {
            printf("find value error!!!\n");
            break;
        }
        else if(((p->Lchild)==NULL)&&((p->Rchild)==NULL))
        {
            res = p->value;
            break;
        }
    }

    return res;
}

int JpegDecoder::ReadFromFile()
{
    current_pos++;
    if(current_pos >= actual_size)
    {
        actual_size = fread(frbuff, 1, frbuff_size, fp);
        if(actual_size <=0)
        {
            printf("Read Nothing!\n");
            return 0;
        }
        current_pos = 0;
    }
    return *(frbuff+current_pos);
}

int JpegDecoder::NextByte()
{
    if (readCount == -1)
    {
        // reset
        readCount = 7;
        //fread(&currentVal, 1, 1, fp);
        currentVal = ReadFromFile();

        // check
        if (currentVal == 0xFF) //标记值
        {
            currentVal = ReadFromFile(); //读取下一个字节
            switch (currentVal)
            {
            case 0x00: currentVal = 0xFF; break;
            case 0xD9: endOfDecoder = true; break;
            case 0xD0:
            case 0xD1:
            case 0xD2:
            case 0xD3:
            case 0xD4:
            case 0xD5:
            case 0xD6:
            case 0xD7: ResetDC(); break;
            default:break;
            };
        }
    }

    if (endOfDecoder) return 0; // end of decoder

    readCount = -1;
    return currentVal;
}