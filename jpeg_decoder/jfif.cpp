#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdbool.h>
#include "jfif.h"
#include "huffman.h"
#include "zigzag.h"
#include "dct.h"
#include <algorithm>
#include <cstring>

using namespace std;

typedef struct {
	int width;
	int height;
	int *databuf;
} YUV;

string int2binarystring(int nowcode, int digit)
{
	string res;
    while (digit>0)
    {
        res.push_back((nowcode & 1) + '0');
        nowcode >>= 1;
        digit -= 1;
    }
	reverse(res.begin(), res.end());
 	return res;
}

void char2binary(char *data, int *stream, int& stream_idx)
{
	int i;
	unsigned char c = data[0];
	unsigned char check = data[0];
	unsigned char _check = data[1];
	for (i = 7; i >= 0; i--)
	{
		stream[i] = c & 1;
		c = c >> 1;
	}
	if(check == FF)
	{
		if(_check == 0)
		{
			stream_idx = stream_idx + 2;
		}
		else
		{
			stream_idx = stream_idx + 1;
		}
	}
	else
	{
		stream_idx = stream_idx + 1;
	}
	
}

string get_HufCode(int dc_length, int *bitStream, int& stream_idx, int& bit_idx, char *encodedData)
{
	int i;
	char temp_c;
	string code;
	if(bit_idx + dc_length > 15)
	{
		for(i = bit_idx + 1;i<8;i++)
		{
			temp_c = bitStream[i] + '0';
			code += temp_c;
		}
		char2binary(&encodedData[stream_idx], bitStream, stream_idx);
		for(i = 0;i<8;i++)
		{
			temp_c = bitStream[i] + '0';
			code += temp_c;
		}
		char2binary(&encodedData[stream_idx], bitStream, stream_idx);
		for(i = 0;i<bit_idx + dc_length - 15;i++)
		{
			temp_c = bitStream[i] + '0';
			code += temp_c;
		}
		bit_idx = bit_idx + dc_length - 16; 
	}
	else if(bit_idx + dc_length > 7)
	{
		for(i = bit_idx + 1;i<8;i++)
		{
			temp_c = bitStream[i] + '0';
			code += temp_c;
		}
		char2binary(&encodedData[stream_idx], bitStream, stream_idx);
		for(i = 0;i<bit_idx + dc_length - 7;i++)
		{
			temp_c = bitStream[i] + '0';
			code += temp_c;
		}
		bit_idx = bit_idx + dc_length - 8; 
	}
	else
	{
		for(i = bit_idx + 1;i<bit_idx + dc_length+1;i++)
		{
			temp_c = bitStream[i] + '0';
			code += temp_c;
		}
		bit_idx = bit_idx + dc_length;
	}
	return code;
}

void build_conti_HT(JFIF *jfif, unsigned char *huffmanCode, int dataLen, int lensum) //acdcindex totallength ht ...
{
	int digit;
	int nowcode = 0;
	int value_offset = 17;
	int idx = (huffmanCode[0] & 0x0f) + (huffmanCode[0] & 0xf0) / 8;
	for(digit = 1; digit<17; digit++)
	{
		if(huffmanCode[digit] != 0)
		{
			int i;
			for(i = 0; i<huffmanCode[digit]; i++)
			{
				string code = int2binarystring(nowcode, digit);
				store_huffmancode(idx,code,huffmanCode[value_offset]);
				if(huffmanCode[digit] != i+1)
					nowcode = nowcode + 1;
				value_offset += 1;
			}
			nowcode = nowcode + 1;
		}
		nowcode = nowcode * 2;
	}
	//traversal_huffmantable(idx);
	int now_sum = lensum + value_offset;
	//printf("idx:%d, value_offset:%d\n",idx,now_sum);
	if(now_sum!=dataLen)
	{
		build_conti_HT(jfif, huffmanCode + value_offset, dataLen, now_sum);
	}
	return;
}

void build_HT(JFIF *jfif, unsigned char *huffmanCode) //acdcindex totallength ht ...
{
	int digit;
	int nowcode = 0;
	int value_offset = 19;
	int dataLen = huffmanCode[0] * 256 + huffmanCode[1];
	int idx = (huffmanCode[2] & 0x0f) + (huffmanCode[2] & 0xf0) / 8;
	for(digit = 1; digit<17; digit++)
	{
		if(huffmanCode[digit+2] != 0)
		{
			int i;
			for(i = 0; i<huffmanCode[digit+2]; i++)
			{
				string code = int2binarystring(nowcode, digit);
				store_huffmancode(idx,code,huffmanCode[value_offset]);
				if(huffmanCode[digit+2] != i+1)
					nowcode = nowcode + 1;
				value_offset += 1;
	
			}
			nowcode = nowcode + 1;
		}
		nowcode = nowcode * 2;
	}
	//traversal_huffmantable(idx);
	//printf("idx:%d, value_offset:%d\n",idx,value_offset);
	if(value_offset!=dataLen)
	{
		build_conti_HT(jfif, huffmanCode + value_offset, dataLen, value_offset);
	}
	return;
}

void build_QT(JFIF *jfif, unsigned char *qtData)
{
	int i;
	int length = (qtData[0] << 8) | qtData[1];
	int offset = 2;
	while(offset != length)
	{
		int QTinfo = qtData[offset];
		int idx = QTinfo % 16;
		jfif->QT[idx].id = idx;
		if(QTinfo/16 == 0)
			jfif->QT[idx].precision = 1;
		else
			jfif->QT[idx].precision = 2;
		
		if(jfif->QT[idx].precision == 1)
		{
			for(i=0;i<64;i++)
				jfif->QT[idx].table[i] = qtData[offset + i + 1];
			offset = offset + 65;
		}
		else
		{
			for(i=0;i<64;i++)
				jfif->QT[idx].table[i] = qtData[offset + i * 2 + 1] + qtData[offset + i * 2 + 2];
			offset = offset + 129;
		}
	}
	/*
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
			printf("%d ",jfif->QT[0].table[i*8+j]);
		printf("\n");
	}
	printf("\n");
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
			printf("%d ",jfif->QT[1].table[i*8+j]);
		printf("\n");
	}
	*/
	//printf("QTlength:%d\n",length);
}

JFIF* jfif_load(unsigned char *jpeg_data, long fileSize)
{
	JFIF *jfif = NULL;
    int   ret    =-1;
    long  offset = 0;
    int   i;
	jfif = (JFIF*) malloc(sizeof(JFIF));

	while(jpeg_data[offset] != FF)
	{
		offset += 1;
	}
	offset += 1;

	// SOI
	if(jpeg_data[offset] != SOI)
	{
		printf("The jpg file data is corrupted! - Fail to find SOI.\n");
		exit(0);
	}

	while(true){
		while(jpeg_data[offset] != FF)
		{
			offset += 1;
		}
		offset += 1;

		if(jpeg_data[offset] <= APP15 && jpeg_data[offset] >= APP0) //不理他
		{
			int appLength = (jpeg_data[offset+1] << 8) | (jpeg_data[offset+2] << 0);
			offset = offset + 1 + appLength;
			//printf("appLength:%d\n",appLength);
		}
		else if(jpeg_data[offset] == SOF0)
		{
			jfif->width    = (jpeg_data[offset+6] << 8) | (jpeg_data[offset+7] << 0);
            jfif->height   = (jpeg_data[offset+4] << 8) | (jpeg_data[offset+5] << 0);
            //printf("width:%d, height:%d\n",jfif->width,jfif->height);
            jfif->channelNum = jpeg_data[offset+8];
            offset = offset + 9;
            for(i=0; i<(jfif->channelNum); i++)
            {
            	int idx = jpeg_data[offset] - 1;
            	jfif->channel_info[idx].id = jpeg_data[offset];
            	jfif->channel_info[idx].sampleRate_X = jpeg_data[offset + 1] / 16;
            	jfif->channel_info[idx].sampleRate_Y = jpeg_data[offset + 1] % 16;
            	jfif->channel_info[idx].QT_idx = jpeg_data[offset + 2];
            	offset = offset + 3;
            }
            /*
            printf("channelNum:%d\n",jfif->channelNum);
            for(i=0; i<(jfif->channelNum); i++)
            {
            	printf("channel_ID:%d, sampleRate_X:%d, sampleRate_Y:%d, QT_idx:%d\n",jfif->channel_info[i].id,
            		jfif->channel_info[i].sampleRate_X,jfif->channel_info[i].sampleRate_Y,jfif->channel_info[i].QT_idx);
            }
            */
		}
		else if(jpeg_data[offset] == DHT)
		{
			build_HT(jfif, &jpeg_data[offset+1]);
		}
		else if(jpeg_data[offset] == DQT)
		{
			build_QT(jfif,&jpeg_data[offset+1]);
		}
		else if(jpeg_data[offset] == DRI)
		{
			jfif->Ri = (jpeg_data[offset+3] << 8) | (jpeg_data[offset+4] << 0);
		}
		else if(jpeg_data[offset] == SOS)
		{
			offset = offset + 4;
			for(i=0; i<(jfif->channelNum); i++)
            {
            	int idx = jpeg_data[offset] - 1;
            	jfif->channel_info[idx].HT_DC_idx = jpeg_data[offset + 1] / 16;
            	jfif->channel_info[idx].HT_AC_idx = jpeg_data[offset + 1] % 16;
            	offset = offset + 2;
            }
            /*
            printf("channelNum:%d\n",jfif->channelNum);
            for(i=0; i<(jfif->channelNum); i++)
            {
            	printf("channel_ID:%d, HT_DC_idx:%d, HT_AC_idx:%d\n",jfif->channel_info[i].id,
            		jfif->channel_info[i].HT_DC_idx, jfif->channel_info[i].HT_AC_idx);
            }
			*/
			//printf("fileSize:%d, offset:%d\n",fileSize,offset);
		    jfif->encodedDataLen = (int) fileSize - offset - 3;
		    jfif->encodedData = (char*) malloc(jfif->encodedDataLen);
		    //printf("dataLen:%d\n",jfif->encodedDataLen);
		    memcpy(jfif->encodedData, &jpeg_data[offset + 3], jfif->encodedDataLen);
		    break;
		}
	}
    return jfif;
}

int* jfif_decode(void *_jfif)
{
	JFIF *jfif = (JFIF*)_jfif;
	int channelNum = jfif->channelNum;
	YUV *yuv = NULL;
	yuv = (YUV*) malloc(channelNum*sizeof(YUV));

	//intial decoding data
	int SRX_MAX = 0;
	int SRY_MAX = 0;
	int i, j;
	for(i = 0;i<channelNum;i++)
	{
		if(SRX_MAX < jfif->channel_info[i].sampleRate_X)
			SRX_MAX = jfif->channel_info[i].sampleRate_X;
		if(SRY_MAX < jfif->channel_info[i].sampleRate_Y)
			SRY_MAX = jfif->channel_info[i].sampleRate_Y;
	}
	//printf("SRX_MAX:%d, SRY_MAX:%d\n",SRX_MAX,SRY_MAX);

	int MCUheight 	= SRY_MAX * 8;
	int MCUwidth 	= SRX_MAX * 8;
	//align yh&yw to 2^n
	int yh 		= (jfif->height + MCUheight -1) & ~(MCUheight - 1);
	int yw 		= (jfif->width + MCUwidth -1) & ~(MCUwidth - 1);
	int MCU_Hcount 	= yh / MCUheight;
	int MCU_Wcount 	= yw / MCUwidth;
	for(i = 0;i<channelNum;i++)
	{
		if(i == 0)
		{
			yuv[i].height = yh;
			yuv[i].width = yw;
		}
		else
		{
			yuv[i].height = yh * jfif->channel_info[i].sampleRate_Y / SRY_MAX;
			yuv[i].width = yw * jfif->channel_info[i].sampleRate_X / SRX_MAX;
		}
		yuv[i].databuf = (int*) malloc(yuv[i].height * yuv[i].width * sizeof(int));
		//printf("id:%d, w:%d, h:%d\n",i,yuv[i].width,yuv[i].height);
	}
	//start to decode encoded data block by block
	int block[64];
	int previous_DC[4] = {0};
	int channel_idx;
	int block_idx = 0;
	int stream_idx = 0; // 
	int bit_idx = 0;	// 0 <= bit_idx <= 7
	int *bitStream = (int *)malloc(sizeof(int) * 8);
	char temp_c;
	string nowHufCode;
	nowHufCode.clear();	//clear codestring
	char2binary(&jfif->encodedData[stream_idx], bitStream, stream_idx);
	
	int MCU_idx;
	int x, y, *isrc, *idst;
	//initial cosine table
	initial_DCT_table();
	// decode data to yuv
	for(MCU_idx = 0;MCU_idx<MCU_Hcount*MCU_Wcount;MCU_idx++)
	{
		//printf("MCU_idx:%d, %d\n",MCU_idx/MCU_Wcount,MCU_idx%MCU_Wcount);
		for(channel_idx = 0;channel_idx<channelNum;channel_idx++)
		{
			for(y = 0;y<jfif->channel_info[channel_idx].sampleRate_Y;y++)
			{
				for(x = 0;x<jfif->channel_info[channel_idx].sampleRate_X;x++)
				{
					for (i = 0; i < 64; i++)
						block[i] = 0;
					temp_c = bitStream[bit_idx] + '0';
					nowHufCode += temp_c;
					while(!check_HufCode(nowHufCode, 0, jfif->channel_info[channel_idx].HT_DC_idx))
					{
						if(bit_idx==7)
						{
							bit_idx = 0;
							char2binary(&jfif->encodedData[stream_idx], bitStream, stream_idx);
							temp_c = bitStream[bit_idx] + '0';
							nowHufCode += temp_c;
						}
						else
						{
							bit_idx = bit_idx + 1;
							temp_c = bitStream[bit_idx] + '0';
							nowHufCode += temp_c;
						}
					}
					int DC_code_length = find_HufCode(nowHufCode, 0, jfif->channel_info[channel_idx].HT_DC_idx) % 16;
					nowHufCode = get_HufCode(DC_code_length, bitStream, stream_idx, bit_idx, jfif->encodedData);
					int DC_value = getValue(nowHufCode, DC_code_length);
					DC_value = DC_value + previous_DC[channel_idx];
					previous_DC[channel_idx] = DC_value;
					block[block_idx] = DC_value;
					block_idx = block_idx + 1;
					//printf("DC_Code:%s, DC_length:%d, DC_value:%d, DC_table_idx:%d\n",nowHufCode.c_str(),DC_code_length,DC_value,jfif->channel_info[channel_idx].HT_DC_idx);

					while(block_idx < 64)
					{
						nowHufCode.clear();
						while(!check_HufCode(nowHufCode, 1, jfif->channel_info[channel_idx].HT_AC_idx))
						{
							if(bit_idx==7)
							{
								bit_idx = 0;
								char2binary(&jfif->encodedData[stream_idx], bitStream, stream_idx);
								temp_c = bitStream[bit_idx] + '0';
								nowHufCode += temp_c;
							}
							else
							{
								bit_idx = bit_idx + 1;
								temp_c = bitStream[bit_idx] + '0';
								nowHufCode += temp_c;
							}
						}
						//printf("%s\n",nowHufCode.c_str());
						int AC_code_info = find_HufCode(nowHufCode, 1, jfif->channel_info[channel_idx].HT_AC_idx);
						if(AC_code_info == 0) // 0/0
						{
							break;
						}
						else	// get ac data
						{
							int RunLength = AC_code_info / 16;
							int AC_code_length = AC_code_info % 16;
							nowHufCode = get_HufCode(AC_code_length, bitStream, stream_idx, bit_idx, jfif->encodedData);
							int AC_value = getValue(nowHufCode, AC_code_length);
							block[block_idx + RunLength] = AC_value;
							block_idx = block_idx + RunLength + 1;
						}
					}
					//next block
					if(bit_idx==7)
					{
						bit_idx = 0;
						char2binary(&jfif->encodedData[stream_idx], bitStream, stream_idx);
					}
					else
					{
						bit_idx = bit_idx + 1;
					}
					block_idx = 0;
					nowHufCode.clear();
					//Quantization
					for(i = 0 ;i<64;i++)
					{
						block[i] = block[i] * jfif->QT[jfif->channel_info[channel_idx].QT_idx].table[i];
					}
					//de zigzag
					zigzag_decode(block);
					//idct
					IDCT2_8X8(block);
					//add 128
					for(i = 0;i<64;i++)
					{
						block[i] = block[i] + 128;
					}
					//write to yuv buffer by mcu index
					int now_x, now_y;
					now_x    = ((MCU_idx % MCU_Wcount) * MCUwidth + x * 8) * jfif->channel_info[channel_idx].sampleRate_X / SRX_MAX;
					now_y    = ((MCU_idx / MCU_Wcount) * MCUheight + y * 8) * jfif->channel_info[channel_idx].sampleRate_Y / SRY_MAX;
                    
                    idst = yuv[channel_idx].databuf + now_y * yuv[channel_idx].width + now_x;
                    isrc = &block[0];
                    
                    for (i=0; i<8; i++) {
                        memcpy(idst, isrc, 8 * sizeof(int));
                        idst += yuv[channel_idx].width;
                        isrc += 8;
                    }
				}
			}
		}
	}
	//yuv to rgb
	int *rgb = (int*) malloc(3 * jfif->height * jfif->width * sizeof(int));
	int *r = &rgb[0];
	int *g = &rgb[jfif->height * jfif->width];
	int *b = &rgb[2 * jfif->height * jfif->width];
	int *ysrc, *usrc, *vsrc;
	ysrc = yuv[0].databuf;
	for(i = 0 ;i<jfif->height;i++)
	{
		int uy = i * jfif->channel_info[1].sampleRate_Y / SRY_MAX;
        int vy = i * jfif->channel_info[2].sampleRate_Y / SRY_MAX;
		for(j = 0;j<jfif->width;j++)
		{
			int ux = j * jfif->channel_info[1].sampleRate_X / SRX_MAX;
            int vx = j * jfif->channel_info[2].sampleRate_X / SRX_MAX;
            usrc = yuv[1].databuf + uy * yuv[1].width + ux;
            vsrc = yuv[2].databuf + vy * yuv[2].width + vx;
            //yuv to rgb
            *r = (int)((float)*ysrc + 1.402 * ((float)*vsrc-128));
            *g = (int)((float)*ysrc - 0.34414 * ((float)*usrc - 128) - 0.71414 * ((float)*vsrc - 128));
            *b = (int)((float)*ysrc + 1.772 * ((float)*usrc - 128));
            //printf("r:%d, g:%d, b:%d\n",*r,*g,*b);
            //printf("y:%d, u:%d, v:%d\n",*ysrc,*usrc,*vsrc);
            r 	+= 1;
            g 	+= 1;
            b 	+= 1;
            ysrc+= 1;
		}
		ysrc -= jfif->width;
        ysrc += yuv[0].width;
	}
	
	return rgb;
}