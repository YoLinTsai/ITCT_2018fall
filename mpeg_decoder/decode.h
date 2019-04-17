#pragma once
#include<iostream>
#include<cstring>
#include <chrono>
#include"bitstream.h"
#include"huffman.h"
#include"picture.h"
#include"dct.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std::chrono;

#define I_PICTURE 1
#define P_PICTURE 2
#define B_PICTURE 3
#define Max(a,b) (a>b ? a:b)
#define Min(a,b) (a<b ? a:b)
#define Clip(a) Max(Min(a,255),0)

class MPEG1 {
public:
	MPEG1(const char *filename);
	~MPEG1();
	void decode();
	void decode_seq_header();
	void read_group_of_picture();
	void read_picture();
	void read_slice();
	void read_macroblock(bool first_mb);
	void read_block(int idx);
	void Recon_DCT_Coeff(int block_idx, int *dct_recon, int dct_zz[]);
	Picture *picture;
	void reset_beforeSlice();
	void macroblockType2parameter(int type);
	int sign(int v);
	void set_DC_past(int block_idx, int val);
	void reset_DCpast();
	void fill_block();
	void process_predict_macroblock();
	void set_predict_macroblock();
	void set_predict_forward_macroblock();
	void set_predict_backward_macroblock();
	void rgb2bmp(Picture::Pixel **data, Mat& bmp, int width, int height);

private:
	BITstream *bitstream;
	HuffmanTable *huffmantable;
	DCT *dct;
	uint32_t next_bit;
	int mb_width, mb_height;
	int horizontal_size, vertical_size;
	double pel_aspect_ratio, picture_rate;
	int* IQM;
    int* NON_IQM;
    int pic_idx;
    long last_display_time;
	static const double picture_rate_table[16];
	static const int ZIGZAG[64];
};



