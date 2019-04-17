#include "decode.h"
#include <stdio.h>
#include <stdlib.h>

const double MPEG1::picture_rate_table[16] = {-1, 23.976f, 24.f, 25.f, 29.97f, 30.f, 50.f, 59.94f, 60.f};
const int MPEG1::ZIGZAG[64] =
{
	0, 1, 5, 6, 14, 15, 27, 28,
	2, 4, 7, 13, 16, 26, 29, 42,
	3, 8, 12, 17, 25, 30, 41, 43,
	9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63,
};

MPEG1::MPEG1(const char *filename)
{
	this->bitstream = new BITstream(filename);
	this->huffmantable = new HuffmanTable();
	this->picture = new Picture();
	this->dct = new DCT();
	this->IQM = new int[64] {8, 16, 19, 22, 26, 27, 29, 34,
                                        16, 16, 22, 24, 27, 29, 34, 37,
                                        19, 22, 26, 27, 29, 34, 34, 38,
                                        22, 22, 26, 27, 29, 34, 37, 40,
                                        22, 26, 27, 29, 32, 35, 40, 48,
                                        26, 27, 29, 32, 35, 40, 48, 58,
                                        26, 27, 29, 34, 38, 46, 56, 69,
                                        27, 29, 35, 38, 46, 56, 69, 83};

	this->NON_IQM = new int[64] {16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16};
    this->pic_idx = 1;
    this->last_display_time = 0;
}

MPEG1::~MPEG1()
{	
}

void MPEG1::decode()
{
	//video sequence layer
	this->next_bit = this->bitstream->read_bits(32);
	if(this->next_bit == 0x000001b3)
	{
		this->decode_seq_header();
	}
	this->next_bit = this->bitstream->read_bits(32);
	while(this->next_bit == 0x000001b8)
	{
		this->read_group_of_picture();
	}
}

void MPEG1::decode_seq_header()
{
	//sequence header
	int temp8;
	this->horizontal_size = this->bitstream->read_bits(12);
	this->vertical_size = this->bitstream->read_bits(12);
	this->picture->init_picture_space(this->vertical_size, this->horizontal_size);

	this->mb_width = (this->horizontal_size+15) / 16;
	this->mb_height = (this->vertical_size+15) / 16;
	printf("width:%d, height:%d\n",this->horizontal_size,this->vertical_size);
	this->bitstream->skip(4);
	temp8 = this->bitstream->read_bits(4);
	this->picture_rate = picture_rate_table[temp8];
	this->bitstream->skip(18); //ingnore
	this->bitstream->skip(1);
	this->bitstream->skip(10);
	this->bitstream->skip(1);
	temp8 = this->bitstream->read_bits(1);
	if(temp8 == 1) //load intra_quantizer_table
	{
		for(int i=0;i<64;i++)
		{
			this->IQM[i] = this->bitstream->read_bits(8);
		}
	}
	temp8 = this->bitstream->read_bits(1);
	if(temp8 == 1) //load non_intra_quantizer_table
	{
		for(int i=0;i<64;i++)
		{
			this->NON_IQM[i] = this->bitstream->read_bits(8);
		}
	}
}

void MPEG1::read_group_of_picture()
{
	this->bitstream->skip(25); //time_code
	this->bitstream->skip(1);
	this->bitstream->skip(1);
	this->bitstream->align();

	this->next_bit = this->bitstream->read_bits(32);
	while(this->next_bit == 0x00000100)
	{
		this->read_picture();
	}
	return;
}

void MPEG1::read_picture()
{
	//picture_layer 2.4.2.5
	int temporal_reference = this->bitstream->read_bits(10);
	int picture_coding_type = this->bitstream->read_bits(3);
	int vbv_delay = this->bitstream->read_bits(16);
	printf("PICTURE %d, type:%d\n",temporal_reference,picture_coding_type);
	this->picture->set_picture_coding_type(picture_coding_type);

	if(picture_coding_type == P_PICTURE || picture_coding_type == B_PICTURE)
	{
		int f = this->bitstream->read_bits(4);
		this->picture->full_pel_forward_vector = (f>>3)&1;
		this->picture->forward_r_size = (f&7) - 1;
		this->picture->forward_f = 1 << this->picture->forward_r_size;
	}
	if(picture_coding_type == B_PICTURE)
	{
		int b = this->bitstream->read_bits(4);
		this->picture->full_pel_backward_vector = (b>>3)&1;
		this->picture->backward_r_size = (b&7) - 1;
		this->picture->backward_f = 1 << this->picture->backward_r_size;
	}
	while(this->bitstream->read_bits(1)==1) //extra_bit_picture
	{
		this->bitstream->skip(8); //extra_information_picture
	}
	this->bitstream->align();
	if(picture_coding_type != B_PICTURE)
	{
		
		for(int i=0;i<this->vertical_size;i++)
		{
			for(int j=0;j<this->horizontal_size;j++)
			{
				this->picture->pel_for_past[i][j].R = this->picture->pel_back_past[i][j].R;
				this->picture->pel_for_past[i][j].G = this->picture->pel_back_past[i][j].G;
				this->picture->pel_for_past[i][j].B = this->picture->pel_back_past[i][j].B;
			}
		}
		if(temporal_reference != -1)
		{
			namedWindow("MPEG1", WINDOW_AUTOSIZE); 
			Mat bmp(this->vertical_size, this->horizontal_size, CV_8UC3);
			rgb2bmp(this->picture->pel_for_past, bmp, this->horizontal_size, this->vertical_size);
			long now_time = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
			double mpeg_delay = (1000/this->picture_rate);
			long delay = mpeg_delay - (now_time - last_display_time)+100;
			if(last_display_time>0 && delay>0)
				waitKey(delay);
			imshow("MPEG1", bmp);
			last_display_time = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
		}
	}

	this->next_bit = this->bitstream->read_bits(32);
	while(this->next_bit>=0x00000101 && this->next_bit<0x000001af)
	{
		this->read_slice();
	}

	if(picture_coding_type != B_PICTURE)
	{
		for(int i=0;i<this->vertical_size;i++)
		{
			for(int j=0;j<this->horizontal_size;j++)
			{
				this->picture->pel_back_past[i][j].R = this->picture->data[i][j].R;
				this->picture->pel_back_past[i][j].G = this->picture->data[i][j].G;
				this->picture->pel_back_past[i][j].B = this->picture->data[i][j].B;
			}
		}
	}
	else if(temporal_reference != -1 && picture_coding_type == B_PICTURE)
	{
		namedWindow("MPEG1", WINDOW_AUTOSIZE); 
		Mat bmp(this->vertical_size, this->horizontal_size, CV_8UC3);
		rgb2bmp(this->picture->data, bmp, this->horizontal_size, this->vertical_size);
		long now_time = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
		double mpeg_delay = (1000/this->picture_rate);
		long  delay = mpeg_delay - (now_time - last_display_time);
		if(last_display_time>0 && delay>0)
			waitKey(delay);
		imshow("MPEG1", bmp);
		last_display_time = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
	}

	/*
	if(this->pic_idx%10==0)
	{
		Mat bmp(this->vertical_size, this->horizontal_size, CV_8UC3);
		rgb2bmp(this->picture->data, bmp, this->horizontal_size, this->vertical_size);
		char* Name = new char[20];
		snprintf(Name, sizeof(Name), "%d", this->pic_idx);
		std::strcat(Name,".bmp");
		printf("%s\n",Name);
		imwrite(Name, bmp);
		delete Name;
	}
	*/
	this->pic_idx ++;

}

void MPEG1::read_slice()
{
	this->picture->sliceinfo.slice_vertical_position = (int)(0x000000ff & this->next_bit);
	this->picture->macroblockinfo.quant_scale = (int)this->bitstream->read_bits(5);
	printf("quant_scale:%d\n",this->picture->macroblockinfo.quant_scale);
	while(this->bitstream->read_bits(1)==1) //extra_bit_picture
	{
		this->bitstream->skip(8); //extra_information_picture
	}
	this->reset_beforeSlice();
	bool first_mb = true;
	while(this->bitstream->seek_bits(23)!=0)
	{
		this->read_macroblock(first_mb);
		first_mb = false;
	}
	this->bitstream->align();
	this->next_bit = this->bitstream->read_bits(32);
}

void MPEG1::read_macroblock(bool first_mb)
{
	//printf("macroblock[%d]:\n",this->picture->macroblockinfo.addr);
	//macroblock layer 2.4.2.7
	int AddressIncrement = 0;
	int MacroType;
	int cbp;
	//macroblock_stuffing
	while(this->bitstream->seek_bits(11)==0x0f)
	{
		this->bitstream->skip(11);
	}
	//macroblock_escape
	while(this->bitstream->seek_bits(11)==0x08)
	{
		this->bitstream->skip(11);
		AddressIncrement += 33;
	}
	AddressIncrement += this->huffmantable->getValue(this->huffmantable->MACROBLOCK_ADDRESS_INCREMENT, this->bitstream);
	//printf("AddressIncrement: %d\n",AddressIncrement);
	MacroType = this->huffmantable->getValue(this->huffmantable->MACROBLOCK_TYPE[this->picture->get_picture_coding_type()], this->bitstream);
	//printf("PictureTpye:%d, MacroType: %d\n",this->picture->get_picture_coding_type(),MacroType);

	//process skip macroblock
	if(AddressIncrement>1 && !first_mb)
	{
		for(int i=0;i<AddressIncrement-1;i++)
		{
			this->picture->macroblockinfo.addr += 1;
			for(int j=0; j<6; j++)
			{
				for(int k=0;k<64;k++)
				{
					this->picture->macroblockinfo.dct_recon[j][k] = 0;
				}
			}

			if(this->picture->get_picture_coding_type() == P_PICTURE)
			{
				this->picture->macroblockinfo.intra = 0;
				this->picture->macroblockinfo.recon_down_for = 0;
				this->picture->macroblockinfo.recon_right_for = 0;
				this->picture->macroblockinfo.recon_down_for_prev = 0;
				this->picture->macroblockinfo.recon_right_for_prev = 0;
			}
			this->process_predict_macroblock();
			this->fill_block();
		}
		this->picture->macroblockinfo.addr += 1;
	}
	else
	{
		this->picture->macroblockinfo.addr += AddressIncrement;
	}

	this->macroblockType2parameter(MacroType);

	/*
	if(this->pic_idx==4)
	{
		printf("\tAddressIncrement: %d\n",AddressIncrement);
		printf("\tPictureTpye:%d, MacroType: %d\n",this->picture->get_picture_coding_type(),MacroType);
		printf("\tq:%d, f:%d, b:%d, p:%d, i:%d\n",this->picture->macroblockinfo.quant, 
			this->picture->macroblockinfo.motion_forward, this->picture->macroblockinfo.motion_backward,
			this->picture->macroblockinfo.pattern, this->picture->macroblockinfo.intra);
	}
	*/
	
	if(this->picture->macroblockinfo.quant)
	{
		this->picture->macroblockinfo.quant_scale = this->bitstream->read_bits(5);
	}

	//2.4.2.7
	if(this->picture->macroblockinfo.motion_forward)
	{
		this->picture->macroblockinfo.motion_horizontal_forward_code = this->huffmantable->getValue(this->huffmantable->MOTION_VECTOR, this->bitstream);
		if(this->picture->macroblockinfo.motion_horizontal_forward_code!=0 && this->picture->forward_f!=1)
		{
			this->picture->macroblockinfo.motion_horizontal_forward_r = this->bitstream->read_bits(this->picture->forward_r_size);
		}
		this->picture->macroblockinfo.motion_vertical_forward_code = this->huffmantable->getValue(this->huffmantable->MOTION_VECTOR, this->bitstream);
		if(this->picture->macroblockinfo.motion_vertical_forward_code!=0 && this->picture->forward_f!=1)
		{
			this->picture->macroblockinfo.motion_vertical_forward_r = this->bitstream->read_bits(this->picture->forward_r_size);
		}
	}

	if(this->picture->macroblockinfo.motion_backward)
	{
		this->picture->macroblockinfo.motion_horizontal_backward_code = this->huffmantable->getValue(this->huffmantable->MOTION_VECTOR, this->bitstream);
		if(this->picture->macroblockinfo.motion_horizontal_backward_code!=0 && this->picture->backward_f!=1)
		{
			this->picture->macroblockinfo.motion_horizontal_backward_r = this->bitstream->read_bits(this->picture->backward_r_size);
		}
		this->picture->macroblockinfo.motion_vertical_backward_code = this->huffmantable->getValue(this->huffmantable->MOTION_VECTOR, this->bitstream);
		if(this->picture->macroblockinfo.motion_vertical_backward_code!=0 && this->picture->backward_f!=1)
		{
			this->picture->macroblockinfo.motion_vertical_backward_r = this->bitstream->read_bits(this->picture->backward_r_size);
		}
	}
	
	/*
	if(this->pic_idx==4)
	{
		printf("motion_horizontal_forward_code:%d, motion_horizontal_forward_r:%d\n"
			, this->picture->macroblockinfo.motion_horizontal_forward_code, this->picture->macroblockinfo.motion_horizontal_forward_r);
		printf("motion_vertical_forward_code:%d, motion_vertical_forward_r:%d\n"
			, this->picture->macroblockinfo.motion_vertical_forward_code, this->picture->macroblockinfo.motion_vertical_forward_r);
	}
	*/

	this->set_predict_macroblock();

	this->process_predict_macroblock();

	if(!this->picture->macroblockinfo.intra || AddressIncrement>1)
	{
		this->reset_DCpast();
	}

	cbp = 0;

	if(this->picture->macroblockinfo.pattern)
	{
		cbp = this->huffmantable->getValue(this->huffmantable->MACROBLOCK_PATTERN, this->bitstream);
	}

	if(this->picture->macroblockinfo.intra)
	{
		cbp = 0b111111;
	}

	for(int i=0;i<6;i++)
	{
		for(int j=0;j<64;j++)
		{
			this->picture->macroblockinfo.dct_recon[i][j] = 0;
		}
		if(((cbp >> (5-i))&1) != 0)
		{
			read_block(i);
			//break;
		}
	}

	if(this->picture->macroblockinfo.intra)
	{
		this->picture->sliceinfo.past_intra_addr = this->picture->macroblockinfo.addr;
	}

	if(this->picture->get_picture_coding_type() == B_PICTURE && this->picture->macroblockinfo.intra){
		// B coded picure and last macroblock is intra, reset m.v. for next macroblock
		this->picture->macroblockinfo.recon_down_for = 0;
		this->picture->macroblockinfo.recon_right_for = 0;
		this->picture->macroblockinfo.recon_down_for_prev = 0;
		this->picture->macroblockinfo.recon_right_for_prev = 0;
		this->picture->macroblockinfo.recon_down_back = 0;
		this->picture->macroblockinfo.recon_right_back = 0;
		this->picture->macroblockinfo.recon_down_back_prev = 0;
		this->picture->macroblockinfo.recon_right_back_prev = 0;
	}

	this->fill_block();
}

void MPEG1::read_block(int block_idx)
{
	//printf("\t==BLOCK(%d)==\n",block_idx);
	//Block Layer 2.4.2.8
	int dct_zz[64];
	int run, level;
	int zz_idx = 0;
	memset(dct_zz, 0, sizeof(dct_zz));
	if(this->picture->macroblockinfo.intra)
	{
		int dc_size;
		if(block_idx<4)
		{
			dc_size = this->huffmantable->getValue(this->huffmantable->DCT_DC_SIZE_LUMINANCE, this->bitstream);
			//printf("\t\tDCT_DC_SIZE_LU_minANCE: %d\n",dc_size);
		}
		else
		{
			dc_size = this->huffmantable->getValue(this->huffmantable->DCT_DC_SIZE_CHROMINANCE, this->bitstream);
			//printf("\t\tDCT_DC_SIZE_CHRO_minANCE: %d\n",dc_size);
		}
		if(dc_size!=0)
		{
			//2.4.3.7 dc_differential
			int dc_differential = this->bitstream->read_bits(dc_size);
			//printf("\t\tdct_dc_differential_lu_minance: %d\n",dc_differential);
			if ( dc_differential & (1<<(dc_size-1))) 
				dct_zz[0] = dc_differential ;
			else
				dct_zz[0] = (-1<<(dc_size)) | (dc_differential+1);
		}
	}
	else
	{
		//2.4.3.7 dct_coeff_first
		this->huffmantable->get_DCT_COEFF_FIRST(this->huffmantable->DCT_COEFF, this->bitstream, run, level);
		zz_idx = run;
		dct_zz[run] = level;
	}
	while(this->bitstream->seek_bits(2)!=2)
	{
		this->huffmantable->get_DCT_COEFF_NEXT(this->huffmantable->DCT_COEFF, this->bitstream, run, level);
		zz_idx += run + 1;
		dct_zz[zz_idx] = level;
	}
	//printf("\t\tEOB\n");
	this->bitstream->skip(2); //10 End Of Block
	
	/*
	if(this->pic_idx==6)
	{
		printf("\t\tdct_zz[%d]: ",block_idx);
		for(int i=0;i<64;i++)
		{
			printf("%d ",dct_zz[i]);
		}
		printf("\n");
	}
	*/
	
	this->Recon_DCT_Coeff(block_idx, this->picture->macroblockinfo.dct_recon[block_idx], dct_zz);

	/*
	if(this->pic_idx==3)
	{
		printf("\t\tdct_recon[%d]: ",block_idx);
		for(int m=0;m<64;m++)
		{
			printf("%d ",this->picture->macroblockinfo.dct_recon[block_idx][m]);
		}
		printf("\n");
	}
	*/
	this->dct->IDCT2_8X8(this->picture->macroblockinfo.dct_recon[block_idx]);
	/*
	printf("dct_recon[%d]: ",block_idx);
	for(int m=0;m<64;m++)
	{
		printf("%d ",this->picture->macroblockinfo.dct_recon[block_idx][m]);
	}
	printf("\n");
	*/
	
}

void MPEG1::Recon_DCT_Coeff(int block_idx, int *dct_recon, int dct_zz[])
{
	//Intra-coded Macroblocks 2.4.4.1
	int dc_past = 0;
	if(this->picture->macroblockinfo.intra)
	{
		if(block_idx<4){
			dc_past = this->picture->blockinfo.DC_Y_past;
		}
		else if(block_idx==4){
			dc_past = this->picture->blockinfo.DC_Cb_past;
		}
		else{
			dc_past = this->picture->blockinfo.DC_Cr_past;
		}

		for(int m=0;m<64;m++)
		{
			int i = ZIGZAG[m];
			dct_recon[m] = 
				(2 * dct_zz[i] * this->picture->macroblockinfo.quant_scale * this->IQM[m]) >> 4;
			if((dct_recon[m]&1)==0){
				dct_recon[m] -= this->sign(dct_recon[m]);
			}
			if(dct_recon[m]>2047){
				dct_recon[m] = 2047;
			}
			if(dct_recon[m]<-2048){
				dct_recon[m] = -2048;
			}
		}

		dct_recon[0] = dct_zz[0] * 8;
		if(block_idx>0 && block_idx<4)
		{
			dct_recon[0] += dc_past;
		}
		else
		{
			if(this->picture->macroblockinfo.addr - this->picture->sliceinfo.past_intra_addr > 1){
				dct_recon[0] += 1024;
			}
			else{
				dct_recon[0] += dc_past;
			}
		}
		this->set_DC_past(block_idx, dct_recon[0]);
	}
	else
	{
		for(int m=0;m<64;m++)
		{
			int i = ZIGZAG[m];
			dct_recon[m] = ((2 * dct_zz[i] + this->sign(dct_zz[i])) * this->picture->macroblockinfo.quant_scale * this->NON_IQM[m]) >> 4;
			if((dct_recon[m]&1)==0){
				dct_recon[m] -= this->sign(dct_recon[m]);
			}
			if(dct_recon[m]>2047){
				dct_recon[m] = 2047;
			}
			if(dct_recon[m]<-2048){
				dct_recon[m] = -2048;
			}
			if(dct_zz[i] == 0){
				dct_recon[m] = 0;
			}
			
		}
	}
	
	/*
	if(this->pic_idx == 3)
	{
		printf("\tdc_past:%d\n",dc_past);
		printf("\t\tdct_recon[%d]: ",block_idx);
		for(int m=0;m<64;m++)
		{
			printf("%d ",dct_recon[m]);
		}
		printf("\n");
		printf("\t\tdct_zz[%d]: ",block_idx);
		for(int m=0;m<64;m++)
		{
			printf("%d ",dct_zz[m]);
		}
		printf("\n");
	}
	*/
	
}

void MPEG1::reset_beforeSlice()
{
	this->picture->macroblockinfo.addr = (this->picture->sliceinfo.slice_vertical_position - 1) * this->mb_width - 1;
	this->picture->sliceinfo.past_intra_addr = -2;
	this->picture->reset_DCpast();
	this->picture->reset_MotionVector();
}

void MPEG1::macroblockType2parameter(int type)
{
	this->picture->macroblockinfo.quant = (type>>4)&1;
	this->picture->macroblockinfo.motion_forward = (type>>3)&1;
	this->picture->macroblockinfo.motion_backward = (type>>2)&1;
	this->picture->macroblockinfo.pattern = (type>>1)&1;
	this->picture->macroblockinfo.intra = (type>>0)&1;
	
	/*
	printf("macroblock_type:%d\n",type);
	printf("\tmacroblcok_quant:%d\n",this->picture->macroblockinfo.quant);
	printf("\tmacroblcok_motion_forward:%d\n",this->picture->macroblockinfo.motion_forward);
	printf("\tmacroblcok_motion_backward:%d\n",this->picture->macroblockinfo.motion_backward);
	printf("\tmacroblcok_pattern:%d\n",this->picture->macroblockinfo.pattern);
	printf("\tmacroblcok_intra:%d\n",this->picture->macroblockinfo.intra);
	*/
}

void MPEG1::reset_DCpast()
{
	this->picture->blockinfo.DC_Y_past = 1024;
	this->picture->blockinfo.DC_Cb_past = 1024;
	this->picture->blockinfo.DC_Cr_past = 1024;
}

int MPEG1::sign(int v)
{
	return (v>0) - (v<0);
}

void MPEG1::set_DC_past(int block_idx, int val)
{
	if(block_idx<4){
		this->picture->blockinfo.DC_Y_past = val;
	}
	else if(block_idx==4){
		this->picture->blockinfo.DC_Cb_past = val;
	}
	else{
		this->picture->blockinfo.DC_Cr_past = val;
	}
}

void MPEG1::fill_block()
{
	//fill data array block by block
	int V = (this->picture->macroblockinfo.addr / this->mb_width)<<4;
	int H = (this->picture->macroblockinfo.addr % this->mb_width)<<4;

	for(int i=0;i<16;i++)
	{
		for(int j=0;j<16;j++)
		{
			int v = V+i;
			int h = H+j;
			if(v >= this->vertical_size || h >= this->horizontal_size) 
				continue;
			int l_id = ((i>>3)<<1)+(j>>3);
			int l_addr = ((i&7)<<3)+(j&7);
			int c_addr = ((i>>1)<<3)+(j>>1);
			int Y = this->picture->macroblockinfo.dct_recon[l_id][l_addr];
			int Cb = this->picture->macroblockinfo.dct_recon[4][c_addr];
			int Cr = this->picture->macroblockinfo.dct_recon[5][c_addr];
			int pel_R = this->picture->pel[v][h].R;
			int pel_G = this->picture->pel[v][h].B;
			int pel_B = this->picture->pel[v][h].G;
			if(this->picture->macroblockinfo.intra){
				Cb-=128, Cr-=128;
			}
			double R = Y + 1.28033 * Cb + pel_R;
            double G = Y - 0.21482 * Cr - 0.38059 * Cb + pel_G;
            double B = Y + 2.12798 * Cr + pel_B;

			this->picture->data[v][h].R = Clip(round(R));
			this->picture->data[v][h].B = Clip(round(G));
			this->picture->data[v][h].G = Clip(round(B));

			/*
			if(this->pic_idx==3)
				printf("%d ", this->picture->data[v][h].G);
			*/
		}
		//if(this->pic_idx==3)
			//printf("\n");
	}
	//if(this->pic_idx==2)
		//printf("\n");
}

void MPEG1::process_predict_macroblock()
{
	// from video 2.4.4.2 
	int V = (this->picture->macroblockinfo.addr / this->mb_width)<<4;
	int H = (this->picture->macroblockinfo.addr % this->mb_width)<<4;
	int picture_coding_type = this->picture->get_picture_coding_type();

	/*
	if(this->pic_idx==3)
	{
		printf("V:%d, H:%d\n",V,H);
	}
	*/

	int right_for = this->picture->macroblockinfo.recon_right_for >> 1 ;
	int down_for = this->picture->macroblockinfo.recon_down_for >> 1 ;
	int right_half_for = this->picture->macroblockinfo.recon_right_for - 2*right_for;
	int down_half_for = this->picture->macroblockinfo.recon_down_for - 2*down_for;

	int right_back = this->picture->macroblockinfo.recon_right_back >> 1 ;
	int down_back = this->picture->macroblockinfo.recon_down_back >> 1 ;
	int right_half_back = this->picture->macroblockinfo.recon_right_back - 2*right_back;
	int down_half_back = this->picture->macroblockinfo.recon_down_back - 2*down_back;

	/*
	if(this->pic_idx==3)
	{
		printf("\tright_for:%d, down_for:%d\n",right_for,down_for);
		printf("\tright_hlaf_for:%d, down_half_for:%d\n",right_half_for,down_half_for);
	}
	*/

	for(int i=V; i<V+16; i++)
	{
		for(int j=H; j<H+16; j++)
		{
			if(i >= this->vertical_size || j >= this->horizontal_size) 
				continue;

			if(this->picture->macroblockinfo.intra)
			{  
				this->picture->pel[i][j].R = 0;
				this->picture->pel[i][j].G = 0;
				this->picture->pel[i][j].B = 0;
				continue;
			}

			//forward motion
			int sum_f = 0;
			int cnt_f = 0;
			if(picture_coding_type == P_PICTURE|| (picture_coding_type == B_PICTURE && this->picture->macroblockinfo.motion_forward))
			{
				sum_f += this->picture->pel_for_past[i+down_for][j+right_for].R;
				cnt_f++;
				if(right_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for][j+right_for+1].R;
					cnt_f++;
				}
				if(down_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for+1][j+right_for].R;
					cnt_f++;
				}
				if(right_half_for && down_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for+1][j+right_for+1].R;
					cnt_f++;
				}
			}

			//backward motion
			int sum_b = 0;
			int cnt_b = 0;
			if(picture_coding_type == B_PICTURE && this->picture->macroblockinfo.motion_backward)
			{
				sum_b += this->picture->pel_back_past[i+down_back][j+right_back].R;
				cnt_b++;
				if(right_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back][j+right_back+1].R;
					cnt_b++;
				}
				if(down_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back+1][j+right_back].R;
					cnt_b++;
				}
				if(right_half_back && down_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back+1][j+right_back+1].R;
					cnt_b++;
				}
			}

			if(cnt_f==0)
				sum_f = 0;
			else
				sum_f = round((double)sum_f/cnt_f);

			if(cnt_b==0)
				sum_b = 0;
			else
				sum_b = round((double)sum_b/cnt_b);
			
			//average forward and backward motion
			if(cnt_b && cnt_f) 
				this->picture->pel[i][j].R = round(((double)sum_b+sum_f)/2);
			else if(cnt_b) 
				this->picture->pel[i][j].R = sum_b;
			else if(cnt_f) 
				this->picture->pel[i][j].R = sum_f;
			else 
				this->picture->pel[i][j].R = 0;

			if(pic_idx==3)
			{
				//printf("%d ",sum_f);
			}

			sum_f = 0;
			cnt_f = 0;
			if(picture_coding_type == P_PICTURE|| (picture_coding_type == B_PICTURE && this->picture->macroblockinfo.motion_forward))
			{
				sum_f += this->picture->pel_for_past[i+down_for][j+right_for].G;
				cnt_f++;
				if(right_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for][j+right_for+1].G;
					cnt_f++;
				}
				if(down_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for+1][j+right_for].G;
					cnt_f++;
				}
				if(right_half_for && down_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for+1][j+right_for+1].G;
					cnt_f++;
				}
			}

			//backward motion
			sum_b = 0;
			cnt_b = 0;
			if(picture_coding_type == B_PICTURE && this->picture->macroblockinfo.motion_backward)
			{
				sum_b += this->picture->pel_back_past[i+down_back][j+right_back].G;
				cnt_b++;
				if(right_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back][j+right_back+1].G;
					cnt_b++;
				}
				if(down_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back+1][j+right_back].G;
					cnt_b++;
				}
				if(right_half_back && down_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back+1][j+right_back+1].G;
					cnt_b++;
				}
			}

			if(cnt_f==0)
				sum_f = 0;
			else
				sum_f = round((double)sum_f/cnt_f);

			if(cnt_b==0)
				sum_b = 0;
			else
				sum_b = round((double)sum_b/cnt_b);
			
			//average forward and backward motion
			if(cnt_b && cnt_f) 
				this->picture->pel[i][j].G = round(((double)sum_b+sum_f)/2);
			else if(cnt_b) 
				this->picture->pel[i][j].G = sum_b;
			else if(cnt_f) 
				this->picture->pel[i][j].G = sum_f;
			else 
				this->picture->pel[i][j].G = 0;

			sum_f = 0;
			cnt_f = 0;
			if(picture_coding_type == P_PICTURE|| (picture_coding_type == B_PICTURE && this->picture->macroblockinfo.motion_forward))
			{
				sum_f += this->picture->pel_for_past[i+down_for][j+right_for].B;
				cnt_f++;
				if(right_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for][j+right_for+1].B;
					cnt_f++;
				}
				if(down_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for+1][j+right_for].B;
					cnt_f++;
				}
				if(right_half_for && down_half_for)
				{
					sum_f += this->picture->pel_for_past[i+down_for+1][j+right_for+1].B;
					cnt_f++;
				}
			}

			//backward motion
			sum_b = 0;
			cnt_b = 0;
			if(picture_coding_type == B_PICTURE && this->picture->macroblockinfo.motion_backward)
			{
				sum_b += this->picture->pel_back_past[i+down_back][j+right_back].B;
				cnt_b++;
				if(right_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back][j+right_back+1].B;
					cnt_b++;
				}
				if(down_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back+1][j+right_back].B;
					cnt_b++;
				}
				if(right_half_back && down_half_back)
				{
					sum_b += this->picture->pel_back_past[i+down_back+1][j+right_back+1].B;
					cnt_b++;
				}
			}

			if(cnt_f==0)
				sum_f = 0;
			else
				sum_f = round((double)sum_f/cnt_f);

			if(cnt_b==0)
				sum_b = 0;
			else
				sum_b = round((double)sum_b/cnt_b);
			
			//average forward and backward motion
			if(cnt_b && cnt_f) 
				this->picture->pel[i][j].B = round(((double)sum_b+sum_f)/2);
			else if(cnt_b) 
				this->picture->pel[i][j].B = sum_b;
			else if(cnt_f) 
				this->picture->pel[i][j].B = sum_f;
			else 
				this->picture->pel[i][j].B = 0;

			
		}
	}
	if(this->pic_idx==3)
	{
		//printf("\n");
	}
}

void MPEG1::rgb2bmp(Picture::Pixel **data, Mat& bmp, int width, int height)
{
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			bmp.at<Vec3b>(i,j)[0] = data[i][j].R;
			bmp.at<Vec3b>(i,j)[1] = data[i][j].B;
			bmp.at<Vec3b>(i,j)[2] = data[i][j].G;
		}
	}
}

void MPEG1::set_predict_macroblock()
{
	int picture_coding_type = this->picture->get_picture_coding_type();
	if(this->picture->macroblockinfo.motion_forward)
	{
		this->set_predict_forward_macroblock();
	}
	else
	{
		if(picture_coding_type == P_PICTURE)
		{
			this->picture->macroblockinfo.recon_down_for = 0;
			this->picture->macroblockinfo.recon_right_for = 0;
			this->picture->macroblockinfo.recon_down_for_prev = 0;
			this->picture->macroblockinfo.recon_right_for_prev = 0;
		}
	}

	if(this->picture->macroblockinfo.motion_backward)
	{
		this->set_predict_backward_macroblock();
	}
}

void MPEG1::set_predict_forward_macroblock()
{
	// just copy from 2.4.4.2 

	int forward_f = this->picture->forward_f;
	int motion_horizontal_forward_code = this->picture->macroblockinfo.motion_horizontal_forward_code;
	int motion_vertical_forward_code = this->picture->macroblockinfo.motion_vertical_forward_code;
	int motion_horizontal_forward_r = this->picture->macroblockinfo.motion_horizontal_forward_r;
	int motion_vertical_forward_r = this->picture->macroblockinfo.motion_vertical_forward_r;
	int full_pel_forward_vector = this->picture->full_pel_forward_vector;

	int complement_horizontal_forward_r, complement_vertical_forward_r, right_little, right_big, down_little, down_big, new_vector;

	if (forward_f == 1 || motion_horizontal_forward_code == 0) 
	{ 
		complement_horizontal_forward_r = 0; 
	} 
	else 
	{ 
		complement_horizontal_forward_r = forward_f - 1 - motion_horizontal_forward_r; 
	}
	
	if (forward_f == 1 || motion_vertical_forward_code == 0) 
	{ 
		complement_vertical_forward_r = 0; 
	} 
	else
	{ 
		complement_vertical_forward_r = forward_f - 1 - motion_vertical_forward_r; 
	}
	right_little = motion_horizontal_forward_code * forward_f;
	if (right_little == 0) 
	{ 
		right_big = 0; 
	} 
	else 
	{ 
		if (right_little > 0) 
		{ 
			right_little = right_little - complement_horizontal_forward_r ; 
			right_big = right_little - 32 * forward_f; 
		} 
		else 
		{ 
			right_little = right_little + complement_horizontal_forward_r ; 
			right_big = right_little + 32 * forward_f; 
		} 
	}
	down_little = motion_vertical_forward_code * forward_f;
	if(down_little == 0)
	{
		down_big = 0; 
	} 
	else 
	{ 
		if(down_little > 0) 
		{ 
			down_little = down_little - complement_vertical_forward_r; 
			down_big = down_little - 32 * forward_f; 
		} 
		else 
		{ 
			down_little = down_little + complement_vertical_forward_r; 
			down_big = down_little + 32 * forward_f; 
		} 
	}

	int _max = (forward_f << 4) - 1;
	int _min = -(forward_f << 4);
	new_vector = this->picture->macroblockinfo.recon_right_for_prev + right_little ;
	
	if(new_vector <= _max && new_vector >= _min)
	{
		this->picture->macroblockinfo.recon_right_for = this->picture->macroblockinfo.recon_right_for_prev + right_little ;
	}
	else
	{
		this->picture->macroblockinfo.recon_right_for = this->picture->macroblockinfo.recon_right_for_prev + right_big ;
	}
	this->picture->macroblockinfo.recon_right_for_prev = this->picture->macroblockinfo.recon_right_for ;

	if(full_pel_forward_vector)
	{	
		this->picture->macroblockinfo.recon_right_for = this->picture->macroblockinfo.recon_right_for << 1 ;
	}

	new_vector = this->picture->macroblockinfo.recon_down_for_prev + down_little;
	if(new_vector <= _max && new_vector >= _min)
	{
		this->picture->macroblockinfo.recon_down_for = this->picture->macroblockinfo.recon_down_for_prev + down_little ;
	}
	else
	{
		this->picture->macroblockinfo.recon_down_for = this->picture->macroblockinfo.recon_down_for_prev + down_big ;
	}
	this->picture->macroblockinfo.recon_down_for_prev = this->picture->macroblockinfo.recon_down_for;

	if(full_pel_forward_vector)
	{
		this->picture->macroblockinfo.recon_down_for = this->picture->macroblockinfo.recon_down_for << 1 ;
	}
	
	/*
	if(this->pic_idx==4)
	{
		printf("forward_f:%d\n",forward_f);
		printf("max:%d, min:%d\n",_max,_min);
		printf("recon_right_for:%d, recon_down_for:%d\n",this->picture->macroblockinfo.recon_right_for,this->picture->macroblockinfo.recon_down_for);
	}
	*/
	
}

void MPEG1::set_predict_backward_macroblock()
{
	// same as forward
	int backward_f = this->picture->backward_f;
	int motion_horizontal_backward_code = this->picture->macroblockinfo.motion_horizontal_backward_code;
	int motion_vertical_backward_code = this->picture->macroblockinfo.motion_vertical_backward_code;
	int motion_horizontal_backward_r = this->picture->macroblockinfo.motion_horizontal_backward_r;
	int motion_vertical_backward_r = this->picture->macroblockinfo.motion_vertical_backward_r;
	int full_pel_backward_vector = this->picture->full_pel_backward_vector;

	int complement_horizontal_backward_r, complement_vertical_backward_r, right_little, right_big, down_little, down_big, new_vector;

	if (backward_f == 1 || motion_horizontal_backward_code == 0) 
	{ 
		complement_horizontal_backward_r = 0; 
	} 
	else 
	{ 
		complement_horizontal_backward_r = backward_f - 1 - motion_horizontal_backward_r; 
	}
	
	if (backward_f == 1 || motion_vertical_backward_code == 0) 
	{ 
		complement_vertical_backward_r = 0; 
	} 
	else
	{ 
		complement_vertical_backward_r = backward_f - 1 - motion_vertical_backward_r; 
	}
	right_little = motion_horizontal_backward_code * backward_f;
	if (right_little == 0) 
	{ 
		right_big = 0; 
	} 
	else 
	{ 
		if (right_little > 0) 
		{ 
			right_little = right_little - complement_horizontal_backward_r ; 
			right_big = right_little - 32 * backward_f; 
		} 
		else 
		{ 
			right_little = right_little + complement_horizontal_backward_r ; 
			right_big = right_little + 32 * backward_f; 
		} 
	}
	down_little = motion_vertical_backward_code * backward_f;
	if(down_little == 0)
	{
		down_big = 0; 
	} 
	else 
	{ 
		if(down_little > 0) 
		{ 
			down_little = down_little - complement_vertical_backward_r; 
			down_big = down_little - 32 * backward_f; 
		} 
		else 
		{ 
			down_little = down_little + complement_vertical_backward_r; 
			down_big = down_little + 32 * backward_f; 
		} 
	}

	int _max = (backward_f << 4) - 1;
	int _min = -(backward_f << 4);
	new_vector = this->picture->macroblockinfo.recon_right_back_prev + right_little ;
	
	if(new_vector <= _max && new_vector >= _min)
	{
		this->picture->macroblockinfo.recon_right_back = this->picture->macroblockinfo.recon_right_back_prev + right_little ;
	}
	else
	{
		this->picture->macroblockinfo.recon_right_back = this->picture->macroblockinfo.recon_right_back_prev + right_big ;
	}
	this->picture->macroblockinfo.recon_right_back_prev = this->picture->macroblockinfo.recon_right_back ;

	if(full_pel_backward_vector)
	{	
		this->picture->macroblockinfo.recon_right_back = this->picture->macroblockinfo.recon_right_back << 1 ;
	}

	new_vector = this->picture->macroblockinfo.recon_down_back_prev + down_little;
	if(new_vector <= _max && new_vector >= _min)
	{
		this->picture->macroblockinfo.recon_down_back = this->picture->macroblockinfo.recon_down_back_prev + down_little ;
	}
	else
	{
		this->picture->macroblockinfo.recon_down_back = this->picture->macroblockinfo.recon_down_back_prev + down_big ;
	}
	this->picture->macroblockinfo.recon_down_back_prev = this->picture->macroblockinfo.recon_down_back;

	if(full_pel_backward_vector)
	{
		this->picture->macroblockinfo.recon_down_back = this->picture->macroblockinfo.recon_down_back << 1 ;
	}
}