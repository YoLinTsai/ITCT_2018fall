#pragma once
#include <stdlib.h>

class Picture{
public:
	class SliceInfo{
	public:
		int slice_vertical_position;
		int past_intra_addr;
		int addr_prev;
	};

	class MacroBlockInfo{
	public:
		int addr;
		int quant_scale;
		int recon_right_for, recon_down_for;
		int recon_right_back, recon_down_back;
		int recon_right_for_prev, recon_down_for_prev;
		int recon_right_back_prev, recon_down_back_prev;

		int quant;
		int motion_forward, motion_backward;
		int pattern;
		int intra;
		
		int motion_horizontal_forward_code;
		int motion_horizontal_forward_r;
		int motion_vertical_forward_code;
		int motion_vertical_forward_r;
		int motion_horizontal_backward_code;
		int motion_horizontal_backward_r;
		int motion_vertical_backward_code;
		int motion_vertical_backward_r;

		int** dct_recon;
	};

	class BlockInfo{
	public:
		int DC_Y_past;
		int DC_Cb_past;
		int DC_Cr_past;
	};

	class Pixel{
	public:
		int R, B, G;
	};

	Picture();
	void set_picture_coding_type(int type);
	int get_picture_coding_type();
	void init_picture_space();
	void reset_DCpast();
	void reset_MotionVector();
	void init_picture_space(int vsize, int hsize);

	int full_pel_forward_vector, forward_f, forward_r_size;
	int full_pel_backward_vector, backward_f, backward_r_size;
	SliceInfo sliceinfo;
	MacroBlockInfo macroblockinfo;
	BlockInfo blockinfo;
	Pixel **data;
	Pixel **pel, **pel_for_past, **pel_back_past;
private:
	int picture_coding_type;
};