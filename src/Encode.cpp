/*
 * \file Encode.cpp
 * \编码部分主体
 */

#include "Encode.h"

using namespace std;

float AANS[8] = { 0.353553, 0.254893, 0.270576, 0.300612, 0.353413, 0.449653, 0.652341, 1.276985 };
int sel_chan[32] = { 0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27, 32, 40, 0, 1, 2, 8, 9, 16, 0, 1, 2, 8, 9, 16};

void AAN_1D(float* dataptr)
{
	float32x4_t tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp10, tmp11, tmp12, tmp13 = { 0 };
	float32x4_t z1, z2, z3, z4, z5, z11, z13 = { 0 };

	float32x4_t DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7;

	float32x4_t bear;

	DATA0 = vld1q_f32(dataptr);
	DATA1 = vld1q_f32(dataptr + 4);
	DATA2 = vld1q_f32(dataptr + 8);
	DATA3 = vld1q_f32(dataptr + 12);
	DATA4 = vld1q_f32(dataptr + 16);
	DATA5 = vld1q_f32(dataptr + 20);
	DATA6 = vld1q_f32(dataptr + 24);
	DATA7 = vld1q_f32(dataptr + 28);

	/* AAN算法第一阶变换 */

	tmp0 = vaddq_f32(DATA0, DATA7);
	tmp7 = vsubq_f32(DATA0, DATA7);
	tmp1 = vaddq_f32(DATA1, DATA6);
	tmp6 = vsubq_f32(DATA1, DATA6);
	tmp2 = vaddq_f32(DATA2, DATA5);
	tmp5 = vsubq_f32(DATA2, DATA5);
	tmp3 = vaddq_f32(DATA3, DATA4);
	tmp4 = vsubq_f32(DATA3, DATA4);

	/* Even part 偶数部分 */
	tmp10 = vaddq_f32(tmp0, tmp3);/* phase 2*/
	tmp13 = vsubq_f32(tmp0, tmp3);
	tmp11 = vaddq_f32(tmp1, tmp2);
	tmp12 = vsubq_f32(tmp1, tmp2);

	DATA0 = vaddq_f32(tmp10, tmp11);/* phase 3 */
	DATA4 = vsubq_f32(tmp10, tmp11);

	z1 = vaddq_f32(tmp12, tmp13);
	bear = vmovq_n_f32((double)0.707106781);
	z1 = vmulq_f32(z1, bear);

	DATA2 = vaddq_f32(tmp13, z1);/* phase 5 */
	DATA6 = vsubq_f32(tmp13, z1);

	/* Odd part 奇数部分 */
	tmp10 = vaddq_f32(tmp4, tmp5);/* phase 2 */
	tmp11 = vaddq_f32(tmp5, tmp6);
	tmp12 = vaddq_f32(tmp6, tmp7);

	/* The rotator is modified from fig 4-8 to avoid extra negations. */
	bear = vmovq_n_f32((double)0.382683433);
	z5 = vsubq_f32(tmp10, tmp12);
	z5 = vmulq_f32(z5, bear);

	bear = vmovq_n_f32((double)0.541196100);
	z2 = vmulq_f32(bear, tmp10);
	z2 = vaddq_f32(z2, z5);

	bear = vmovq_n_f32((double)1.306562965);
	z4 = vmulq_f32(bear, tmp12);
	z4 = vaddq_f32(z4, z5);

	bear = vmovq_n_f32((double)0.707106781);
	z3 = vmulq_f32(tmp11, bear);

	z11 = vaddq_f32(tmp7, z3);
	z13 = vsubq_f32(tmp7, z3);

	DATA5 = vaddq_f32(z13, z2);
	DATA3 = vsubq_f32(z13, z2);
	DATA1 = vaddq_f32(z11, z4);
	DATA7 = vsubq_f32(z11, z4);

	float32x4_t aans;

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

	vst1q_f32(dataptr, DATA0);
	vst1q_f32(dataptr + 4, DATA1);
	vst1q_f32(dataptr + 8, DATA2);
	vst1q_f32(dataptr + 12, DATA3);
	vst1q_f32(dataptr + 16, DATA4);
	vst1q_f32(dataptr + 20, DATA5);
	vst1q_f32(dataptr + 24, DATA6);
	vst1q_f32(dataptr + 28, DATA7);
}



void AAN_2D(float* matdata, float* matres)
{
	float matemp[64] = { 0 };
	float arraytemp[32] = { 0 };
	int i, j = 0;
	for (i = 0; i < 8; i += 4)
	{
		for (j = 0; j < 8; j++)
		{
			arraytemp[j * 4] = matdata[i * 8 + j];
			arraytemp[j * 4 + 1] = matdata[(i + 1) * 8 + j];
			arraytemp[j * 4 + 2] = matdata[(i + 2) * 8 + j];
			arraytemp[j * 4 + 3] = matdata[(i + 3) * 8 + j];
		}
		AAN_1D(arraytemp);
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
		AAN_1D(arraytemp);
		for (j = 0; j < 8; j++)
		{
			matres[j * 8 + i] = arraytemp[j * 4];
			matres[j * 8 + i + 1] = arraytemp[j * 4 + 1];
			matres[j * 8 + i + 2] = arraytemp[j * 4 + 2];
			matres[j * 8 + i + 3] = arraytemp[j * 4 + 3];
		}
	}
}

JpegEncoder::JpegEncoder()
	: m_width(0)
	, m_height(0)
	, m_rgbBuffer(0)
{
	//初始化静态表格
	_initHuffmanTables();
	memset(fwbuffer,0,fwbuff_size+4);
	buffcount = 0;
}

JpegEncoder::~JpegEncoder()
{
	clean();
}

void JpegEncoder::clean(void)
{
	if (m_rgbBuffer) delete[] m_rgbBuffer;
	m_rgbBuffer=0;

	m_width=0;
	m_height=0;
}

bool JpegEncoder::readFromBMP(const int width, const int height, unsigned char* buffer)
{
	//清理旧数据
	clean();

	bool successed = false;

	int size = width * height * 3;
	unsigned char* buff = new unsigned char[size];

	for (int i = 0; i < size; i++)
	{
		buff[i] = *(buffer + i);
	}

	m_rgbBuffer = buff;
	m_width = width;
	m_height = height;
	
	successed = true;

	return successed;
}

bool JpegEncoder::encodeToJPG(const char* fileName, int quality_scale, Mat* M, const int mode)
{
	//尚未读取？
	if(m_rgbBuffer==0 || m_width==0 || m_height==0) return false;

	//输出文件
	FILE* fp = fopen(fileName, "wb");
	if(fp==0) return false;

	//初始化量化表
	_initQualityTables(quality_scale);

	//文件头
	_write_jpeg_header(fp);

	short prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0;
	int newordPos=15;
	unsigned short neword=0;
	
	for(int yPos=0; yPos<m_height; yPos+=16)
	{
		for (int xPos=0; xPos<m_width; xPos+=16)
		{
			
			signed char yData0[64],yData1[64],yData2[64],yData3[64], cbData0[64], cbData1[64], cbData2[64], cbData3[64], crData0[64], crData1[64], crData2[64], crData3[64], mixcb[64], mixcr[64];
			short yQuant[64], cbQuant[64], crQuant[64];
			float y0DCT[64], y1DCT[64], y2DCT[64], y3DCT[64], cbDCT[64], crDCT[64];

			//转换颜色空间
			_convertColorSpace(xPos, yPos, yData0, cbData0, crData0);                    //tranfer block0
			_convertColorSpace(xPos+8, yPos, yData1, cbData1, crData1);                  //tranfer block1
			_convertColorSpace(xPos, yPos+8, yData2, cbData2, crData2);                  //tranfer block2
			_convertColorSpace(xPos+8, yPos+8, yData3, cbData3, crData3);                //tranfer block3

			//mix cb and cr
			for(int mnum=0; mnum<4; mnum++)
			{
				int xoffset = (mnum/2)*4;                      //x offset, row
				int yoffset = (mnum%2)*4;                      //y offset, colunm
				for(int i=0; i<4; i++)
				{
					for(int j=0; j<4; j++)
					{
						int xm = xoffset+i;
						int ym = yoffset+j;
						int mpos = xm*8+ym;
						int xs = i*2;
						int ys = j*2;
						int spos = xs*8+ys;
						if(mnum == 0)                                    //cb,cr sampling
						{
							mixcb[mpos] = cbData0[spos];
							mixcr[mpos] = crData0[spos];
						}
						else if(mnum == 1)
						{
							mixcb[mpos] = cbData1[spos];
							mixcr[mpos] = crData1[spos];
						}
						else if(mnum == 2)
						{
							mixcb[mpos] = cbData2[spos];
							mixcr[mpos] = crData2[spos];
						}
						else if(mnum == 3)
						{
							mixcb[mpos] = cbData3[spos];
							mixcr[mpos] = crData3[spos];
						}
					}
				}
			}

			BitString outputBitString[128];
			int bitStringCounts;

			//Y通道压缩
			_foword_FDC(yData0, yQuant, y0DCT);
			_doHuffmanEncoding(yQuant, prev_DC_Y, m_Y_DC_Huffman_Table, m_Y_AC_Huffman_Table, outputBitString, bitStringCounts); 
			_write_bitstring_(outputBitString, bitStringCounts, neword, newordPos, fp);

			_foword_FDC(yData1, yQuant, y1DCT);
			_doHuffmanEncoding(yQuant, prev_DC_Y, m_Y_DC_Huffman_Table, m_Y_AC_Huffman_Table, outputBitString, bitStringCounts); 
			_write_bitstring_(outputBitString, bitStringCounts, neword, newordPos, fp);

			_foword_FDC(yData2, yQuant, y2DCT);
			_doHuffmanEncoding(yQuant, prev_DC_Y, m_Y_DC_Huffman_Table, m_Y_AC_Huffman_Table, outputBitString, bitStringCounts); 
			_write_bitstring_(outputBitString, bitStringCounts, neword, newordPos, fp);

			_foword_FDC(yData3, yQuant, y3DCT);
			_doHuffmanEncoding(yQuant, prev_DC_Y, m_Y_DC_Huffman_Table, m_Y_AC_Huffman_Table, outputBitString, bitStringCounts); 
			_write_bitstring_(outputBitString, bitStringCounts, neword, newordPos, fp);

			//Cb通道压缩
			_foword_FDC(mixcb, cbQuant, cbDCT);			
			_doHuffmanEncoding(cbQuant, prev_DC_Cb, m_CbCr_DC_Huffman_Table, m_CbCr_AC_Huffman_Table, outputBitString, bitStringCounts);
			_write_bitstring_(outputBitString, bitStringCounts, neword, newordPos, fp);

			//Cr通道压缩
			_foword_FDC(mixcr, crQuant, crDCT);			
			_doHuffmanEncoding(crQuant, prev_DC_Cr, m_CbCr_DC_Huffman_Table, m_CbCr_AC_Huffman_Table, outputBitString, bitStringCounts);
			_write_bitstring_(outputBitString, bitStringCounts, neword, newordPos, fp);

			if(mode == 1)
			{
				//传输DCT系数
				int c = 0;
				for (; c < 20; c++)
	            {
	                //给M的每一个元素赋值
	                M->at<Vec32f>(yPos / 8, xPos / 8)[c] = y0DCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8, xPos / 8 + 1)[c] = y1DCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8 + 1, xPos / 8)[c] = y2DCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8 + 1, xPos / 8 + 1)[c] = y3DCT[sel_chan[c]];
	            }
	            for (; c < 26; c++)
	            {
	                //给M的每一个元素赋值
	                M->at<Vec32f>(yPos / 8, xPos / 8)[c] = cbDCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8, xPos / 8 + 1)[c] = cbDCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8 + 1, xPos / 8)[c] = cbDCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8 + 1, xPos / 8 + 1)[c] = cbDCT[sel_chan[c]];
	            }
	            for (; c < 32; c++)
	            {
	                //给M的每一个元素赋值
	                M->at<Vec32f>(yPos / 8, xPos / 8)[c] = crDCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8, xPos / 8 + 1)[c] = crDCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8 + 1, xPos / 8)[c] = crDCT[sel_chan[c]];
	                M->at<Vec32f>(yPos / 8 + 1, xPos / 8 + 1)[c] = crDCT[sel_chan[c]];
	            }
			}
		}
	}
	
    if(newordPos>=0)
    {
    	push2buff(neword,fp,1);
		newordPos = 15;
		neword = 0;
    }

	_write_word_(0xFFD9, fp); //Write End of Image Marker   
	
	fclose(fp);

	return true;
}

void JpegEncoder::_initHuffmanTables(void)
{
	memset(&m_Y_DC_Huffman_Table, 0, sizeof(m_Y_DC_Huffman_Table));
	_computeHuffmanTable(Standard_DC_Luminance_NRCodes, Standard_DC_Luminance_Values, m_Y_DC_Huffman_Table);

	memset(&m_Y_AC_Huffman_Table, 0, sizeof(m_Y_AC_Huffman_Table));
	_computeHuffmanTable(Standard_AC_Luminance_NRCodes, Standard_AC_Luminance_Values, m_Y_AC_Huffman_Table);

	memset(&m_CbCr_DC_Huffman_Table, 0, sizeof(m_CbCr_DC_Huffman_Table));
	_computeHuffmanTable(Standard_DC_Chrominance_NRCodes, Standard_DC_Chrominance_Values, m_CbCr_DC_Huffman_Table);

	memset(&m_CbCr_AC_Huffman_Table, 0, sizeof(m_CbCr_AC_Huffman_Table));
	_computeHuffmanTable(Standard_AC_Chrominance_NRCodes, Standard_AC_Chrominance_Values, m_CbCr_AC_Huffman_Table);
}

BitString JpegEncoder::_getBitCode(int value)             //computation to gain bitwidth and code
{
	BitString ret;
	int v = (value>0) ? value : -value;
	
	//bit 的长度
	int length = 0;
	for(length=0; v; v>>=1) length++;

	ret.value = value>0 ? value : ((1<<length)+value-1);
	ret.length = length;

	return ret;
};

void JpegEncoder::_initQualityTables(int quality_scale)
{
	if(quality_scale<=0) quality_scale=1;
	if(quality_scale>=100) quality_scale=99;

	for(int i=0; i<64; i++)
	{
		int temp = ((int)(Luminance_Quantization_Table[i] * quality_scale + 50) / 100);
		if (temp<=0) temp = 1;
		if (temp>0xFF) temp = 0xFF;
		m_YTable[ZigZag[i]] = (unsigned char)temp;

		temp = ((int)(Chrominance_Quantization_Table[i] * quality_scale + 50) / 100);
		if (temp<=0) 	temp = 1;
		if (temp>0xFF) temp = 0xFF;
		m_CbCrTable[ZigZag[i]] = (unsigned char)temp;
	}
}

void JpegEncoder::_computeHuffmanTable(const char* nr_codes, const unsigned char* std_table, BitString* huffman_table)      
{                                                                  
	unsigned char pos_in_table = 0;               //to gain huffman table (you can look up one value's bitwidth and corresponding huffman code)
	unsigned short code_value = 0;

	for(int k = 1; k <= 16; k++)
	{
		for(int j = 1; j <= nr_codes[k-1]; j++)
		{
			huffman_table[std_table[pos_in_table]].value = code_value;
			huffman_table[std_table[pos_in_table]].length = k;
			pos_in_table++;
			code_value++;
		}
		code_value <<= 1;
	}  
}

void JpegEncoder::_write_byte_(unsigned char value, FILE* fp)
{
	_write_(&value, 1, fp);
}

void JpegEncoder::_write_word_(unsigned short value, FILE* fp)
{
	unsigned short _value = ((value>>8)&0xFF) | ((value&0xFF)<<8);
	_write_(&_value, 2, fp);
}


void JpegEncoder::_write_special_byte_(unsigned char value, FILE* fp)
{
	_write_byte_(value,fp);
	if(value == 0xff)
		_write_byte_(0x00,fp);
}

void JpegEncoder::_write_special_word_(unsigned short value, FILE* fp)
{
	unsigned char highbyte = 0, lowbyte = 0;
	highbyte = ((value>>8) & 0xff);
	lowbyte = (value & 0xff);
	_write_special_byte_(highbyte,fp);
	_write_special_byte_(lowbyte,fp);
}

void JpegEncoder::_write_(const void* p, int byteSize, FILE* fp)
{
	fwrite(p, 1, byteSize, fp);
}

void JpegEncoder::push2buff(unsigned short value, FILE* fp, int flag)                         //如果为1，则强制清空buffer同时写入数据
{
	unsigned char highbyte = ((value>>8) & 0xff);
	unsigned char lowbyte = (value & 0xff);
	fwbuffer[buffcount] = highbyte;
	buffcount++;
	if(highbyte == 0xff)
	{
		fwbuffer[buffcount] = 0x00;
		buffcount++;
	}
	fwbuffer[buffcount] = lowbyte;
	buffcount++;
	if(lowbyte == 0xff)
	{
		fwbuffer[buffcount] = 0x00;
		buffcount++;
	}
	if((buffcount >= fwbuff_size)||(flag))
	{
		fwrite(fwbuffer, 1, buffcount, fp);
		memset(fwbuffer,0,fwbuff_size+4);
		buffcount = 0;
	}
}

void JpegEncoder::_doHuffmanEncoding(const short* DU, short& prevDC, const BitString* HTDC, const BitString* HTAC, 
	BitString* outputBitString, int& bitStringCounts)
{
	BitString EOB = HTAC[0x00];
	BitString SIXTEEN_ZEROS = HTAC[0xF0];

	int index=0;

	// encode DC
	int dcDiff = (int)(DU[0] - prevDC);
	prevDC = DU[0];

	if (dcDiff == 0) 
		outputBitString[index++] = HTDC[0];
	else
	{
		BitString bs = _getBitCode(dcDiff);

		outputBitString[index++] = HTDC[bs.length];
		outputBitString[index++] = bs;
	}

	// encode ACs
	int endPos=63; //end0pos = first element in reverse order != 0
	while((endPos > 0) && (DU[endPos] == 0)) endPos--;                            //find the last element which is not 0

	for(int i=1; i<=endPos; )
	{
		int startPos = i;
		while((DU[i] == 0) && (i <= endPos)) i++;                                 //find the first element which is no 0

		int zeroCounts = i - startPos;
		if (zeroCounts >= 16)                                                     //if there are more than 16 consecutive zeros
		{
			for (int j=1; j<=zeroCounts/16; j++)                                
				outputBitString[index++] = SIXTEEN_ZEROS;
			zeroCounts = zeroCounts%16;
		}

		BitString bs = _getBitCode(DU[i]);

		outputBitString[index++] = HTAC[(zeroCounts << 4) | bs.length];          //combine the number of zeros and bitwidth
		outputBitString[index++] = bs;
		i++;
	}

	if (endPos != 63)
		outputBitString[index++] = EOB;

	bitStringCounts = index;
}

void JpegEncoder::_write_bitstring_(const BitString* bs, int counts, unsigned short& neword, int& newordPos, FILE* fp)
{
	unsigned short mask[] = {1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

	for(int i=0; i<counts;i++)
	{
		unsigned short value = (unsigned short)bs[i].value;
	    int length = bs[i].length;
	    int leftspace = newordPos + 1;

	    if(leftspace <= length)
	    {
	    	//printf("pre neword = %hX leftspace = %d          value = %hX length = %d",neword,leftspace,value,length);
	    	neword = (neword | ((value>>(length-leftspace)) & mask[leftspace-1]));
	    	//printf("     now neword = %hX          write\n",neword);
	    	push2buff(neword,fp,0);
	    	newordPos = 15 + leftspace - length; 
	    	neword = (value << (newordPos+1));
	    }
	    else
	    {
	    	//printf("pre neword = %hX leftspace = %d          value = %hX length = %d",neword,leftspace,value,length);
	    	neword = (neword | ((value<<(leftspace - length)) & mask[leftspace-1]));
	    	//printf("     now neword = %hX\n",neword);
	    	newordPos -= length;
	    }
	}
}

void JpegEncoder::_convertColorSpace(int xPos, int yPos, signed char* yData, signed char* cbData, signed char* crData)
{
	for (int y=0; y<8; y++)
	{
		unsigned char* p = m_rgbBuffer + (y+yPos)*m_width*3 + xPos*3;
		for (int x=0; x<8; x++)
		{
			unsigned char B = *p++;
			unsigned char G = *p++;
			unsigned char R = *p++;

			yData[y*8+x] = (int)(0.299f * R + 0.587f * G + 0.114f * B - 128);
			cbData[y*8+x] = (int)(-0.1687f * R - 0.3313f * G + 0.5f * B );
			crData[y*8+x] = (int)(0.5f * R - 0.4187f * G - 0.0813f * B);
		}
	}
}

void JpegEncoder::_foword_FDC(const signed char* channel_data, short* fdc_data, float* dct_data)
{
    float data[64] = {0};
    int i = 0;
    for(i=0;i<64;i++)
    {
    	data[i] = channel_data[i];
    }
    AAN_2D(data, dct_data);
    float temp = 0.f;
    for(i=0;i<64;i++)
    {
    	temp = dct_data[i]/m_YTable[ZigZag[i]];
    	fdc_data[ZigZag[i]] = (short) ((short)(temp + 16384.5) - 16384);
    }
}

void JpegEncoder::_write_jpeg_header(FILE* fp)
{
	//SOI
	_write_word_(0xFFD8, fp);		// marker = 0xFFD8

	//APPO
	_write_word_(0xFFE0,fp);		// marker = 0xFFE0
	_write_word_(16, fp);			// length = 16 for usual JPEG, no thumbnail
	_write_("JFIF", 5, fp);			// 'JFIF\0'
	_write_byte_(1, fp);			// version_hi
	_write_byte_(1, fp);			// version_low
	_write_byte_(0, fp);			// xyunits = 0 no units, normal density
	_write_word_(1, fp);			// xdensity
	_write_word_(1, fp);			// ydensity
	_write_byte_(0, fp);			// thumbWidth
	_write_byte_(0, fp);			// thumbHeight

	//DQT
	_write_word_(0xFFDB, fp);		//marker = 0xFFDB
	_write_word_(132, fp);			//size=132
	_write_byte_(0, fp);			//QTYinfo== 0:  bit 0..3: number of QT = 0 (table for Y) 
									//				bit 4..7: precision of QT
									//				bit 8	: 0
	_write_(m_YTable, 64, fp);		//YTable
	_write_byte_(1, fp);			//QTCbinfo = 1 (quantization table for Cb,Cr)
	_write_(m_CbCrTable, 64, fp);	//CbCrTable

	//SOFO
	_write_word_(0xFFC0, fp);			//marker = 0xFFC0
	_write_word_(17, fp);				//length = 17 for a truecolor YCbCr JPG
	_write_byte_(8, fp);				//precision = 8: 8 bits/sample 
	_write_word_(m_height&0xFFFF, fp);	//height
	_write_word_(m_width&0xFFFF, fp);	//width
	_write_byte_(3, fp);				//nrofcomponents = 3: We encode a truecolor JPG

	_write_byte_(1, fp);				//IdY = 1
	_write_byte_(0x22, fp);				//HVY sampling factors for Y (bit 0-3 vert., 4-7 hor.)(SubSamp 1x1)
	_write_byte_(0, fp);				//QTY  Quantization Table number for Y = 0

	_write_byte_(2, fp);				//IdCb = 2
	_write_byte_(0x11, fp);				//HVCb = 0x11(SubSamp 1x1)
	_write_byte_(1, fp);				//QTCb = 1

	_write_byte_(3, fp);				//IdCr = 3
	_write_byte_(0x11, fp);				//HVCr = 0x11 (SubSamp 1x1)
	_write_byte_(1, fp);				//QTCr Normally equal to QTCb = 1
	
	//DHT
	_write_word_(0xFFC4, fp);		//marker = 0xFFC4
	_write_word_(0x01A2, fp);		//length = 0x01A2
	_write_byte_(0, fp);			//HTYDCinfo bit 0..3	: number of HT (0..3), for Y =0
									//			bit 4		: type of HT, 0 = DC table,1 = AC table
									//			bit 5..7	: not used, must be 0
	_write_(Standard_DC_Luminance_NRCodes, sizeof(Standard_DC_Luminance_NRCodes), fp);	//DC_L_NRC
	_write_(Standard_DC_Luminance_Values, sizeof(Standard_DC_Luminance_Values), fp);		//DC_L_VALUE
	_write_byte_(0x10, fp);			//HTYACinfo
	_write_(Standard_AC_Luminance_NRCodes, sizeof(Standard_AC_Luminance_NRCodes), fp);
	_write_(Standard_AC_Luminance_Values, sizeof(Standard_AC_Luminance_Values), fp); //we'll use the standard Huffman tables
	_write_byte_(0x01, fp);			//HTCbDCinfo
	_write_(Standard_DC_Chrominance_NRCodes, sizeof(Standard_DC_Chrominance_NRCodes), fp);
	_write_(Standard_DC_Chrominance_Values, sizeof(Standard_DC_Chrominance_Values), fp);
	_write_byte_(0x11, fp);			//HTCbACinfo
	_write_(Standard_AC_Chrominance_NRCodes, sizeof(Standard_AC_Chrominance_NRCodes), fp);
	_write_(Standard_AC_Chrominance_Values, sizeof(Standard_AC_Chrominance_Values), fp);

	//SOS
	_write_word_(0xFFDA, fp);		//marker = 0xFFC4
	_write_word_(12, fp);			//length = 12
	_write_byte_(3, fp);			//nrofcomponents, Should be 3: truecolor JPG

	_write_byte_(1, fp);			//Idy=1
	_write_byte_(0, fp);			//HTY	bits 0..3: AC table (0..3)
									//		bits 4..7: DC table (0..3)
	_write_byte_(2, fp);			//IdCb
	_write_byte_(0x11, fp);			//HTCb

	_write_byte_(3, fp);			//IdCr
	_write_byte_(0x11, fp);			//HTCr

	_write_byte_(0, fp);			//Ss not interesting, they should be 0,63,0
	_write_byte_(0x3F, fp);			//Se
	_write_byte_(0, fp);			//Bf
}