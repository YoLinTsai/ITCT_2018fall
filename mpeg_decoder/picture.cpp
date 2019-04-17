#include "picture.h"

Picture::Picture()
{
	this->macroblockinfo.dct_recon = new int*[6];
	for(int i=0;i<6;i++)
	{
		this->macroblockinfo.dct_recon[i] = new int[64];
	}
}

void Picture::init_picture_space(int vsize, int hsize)
{
	this->data = new Pixel*[vsize+1];
	this->pel = new Pixel*[vsize+1];
	this->pel_for_past = new Pixel*[vsize+1];
	this->pel_back_past = new Pixel*[vsize+1];
	for(int i=0;i<vsize;i++)
	{
		this->data[i] = new Pixel[hsize];
		this->pel[i] = new Pixel[hsize];
		this->pel_for_past[i] = new Pixel[hsize];
		this->pel_back_past[i] = new Pixel[hsize];
	}
}

void Picture::set_picture_coding_type(int type)
{
	this->picture_coding_type = type;
}

int Picture::get_picture_coding_type()
{
	return this->picture_coding_type;
}

void Picture::reset_DCpast()
{
	this->blockinfo.DC_Y_past = 1024;
	this->blockinfo.DC_Cb_past = 1024;
	this->blockinfo.DC_Cr_past = 1024;
}

void Picture::reset_MotionVector()
{
	this->macroblockinfo.recon_down_for_prev = 0;
	this->macroblockinfo.recon_right_for_prev = 0;
	this->macroblockinfo.recon_right_back_prev = 0;
	this->macroblockinfo.recon_down_back_prev = 0;
}