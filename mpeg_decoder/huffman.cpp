#include "huffman.h"
#include <stdio.h>
#include <string.h>

HuffmanTable::HuffmanTable()
{
	this->init_all_table();
}

void HuffmanTable::init_all_table()
{
	this->MACROBLOCK_TYPE_I = new int[8];
	this->MACROBLOCK_TYPE_P = new int[128];
	this->MACROBLOCK_TYPE_B = new int[128];
	this->MACROBLOCK_ADDRESS_INCREMENT = new int[4096];
	this->MOTION_VECTOR = new int[4096];
	this->MACROBLOCK_PATTERN = new int[1024];
	this->DCT_DC_SIZE_LUMINANCE = new int[256];
	this->DCT_DC_SIZE_CHROMINANCE = new int[512];
	this->DCT_COEFF = new int[262144];

	this->init_MACROBLOCK_ADDRESS_INCREMENT();
	this->init_MACROBLOCK_TYPE();
	this->init_MOTION_VECTOR();
	this->init_MACROBLOCK_PATTERN();
	this->init_DCT_COEFFIECIENTS();
	this->init_VARIABLE_LENGTH_CODE();
}

int HuffmanTable::getValue(int* table, BITstream *bitstream)
{
	int key = 1;
	while(1){
		key = (key << 1) | bitstream->read_bits(1);
		int it = table[key];
		//printf("%d",bitstream->read_bits(1));
		if(it!=-17)
		{
			//printf("\n");
			return it;
		}
	}
}

void HuffmanTable::get_DCT_COEFF_FIRST(int* table, BITstream *bitstream, int& run, int& level)
{
	int key = 1;
	while(1){
		key = (key << 1) | bitstream->read_bits(1);
		int it = table[key];
		if(it!=-17)
		{
			run = it>>8;
			int neg = (key&0x01);
			if(!neg){
				level = it&0xFF;
			}
			else{
				level = (-1)*(it&0xFF);
			}
			return;
		}
		else if(key == 0b110)
		{ 
			run = 0;
			level = 1;
			return;
		}
		else if(key == 0b111)
		{ 
			run = 0;
			level = -1;
			return;
		}
		else if(key == 0b1000001)
		{
			run = bitstream->read_bits(6);
			level = (char)bitstream->read_bits(8);
			if(level==0||level==-128) 
			{
				level = (level<<1)|bitstream->read_bits(8);
			}
			return;
		}
	}
}

void HuffmanTable::get_DCT_COEFF_NEXT(int* table, BITstream *bitstream, int& run, int& level)
{
	int key = 1;
	while(1){
		key = (key << 1) | bitstream->read_bits(1);
		int it = table[key];
		if(it!=-17)
		{

			run = it>>8;
			int neg = (key&0x01);
			if(!neg){
				level = it&0xFF;
			}
			else{
				level = (-1)*(it&0xFF);
			}
			return;
		}
		else if(key == 0b1110)
		{ 
			run = 0;
			level = 1;
			return;
		}
		else if(key == 0b1111)
		{ 
			run = 0;
			level = -1;
			return;
		}
		else if(key == 0b1000001)
		{
			run = bitstream->read_bits(6);
			level = (char)bitstream->read_bits(8);
			if(level==0||level==-128) 
			{
				level = (level<<1)|bitstream->read_bits(8);
			}
			return;
		}
	}
}

void HuffmanTable::init_MACROBLOCK_ADDRESS_INCREMENT()
{
	for(int i=0;i<4096;i++)
	{
		MACROBLOCK_ADDRESS_INCREMENT[i] = -17;
	}
	MACROBLOCK_ADDRESS_INCREMENT[0b11] = 1;
	MACROBLOCK_ADDRESS_INCREMENT[0b1011] = 2;
	MACROBLOCK_ADDRESS_INCREMENT[0b1010] = 3;
	MACROBLOCK_ADDRESS_INCREMENT[0b10011] = 4;
	MACROBLOCK_ADDRESS_INCREMENT[0b10010] = 5;
	MACROBLOCK_ADDRESS_INCREMENT[0b100011] = 6;
	MACROBLOCK_ADDRESS_INCREMENT[0b100010] = 7;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000111] = 8;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000110] = 9;
	MACROBLOCK_ADDRESS_INCREMENT[0b100001011] = 10;
	MACROBLOCK_ADDRESS_INCREMENT[0b100001010] = 11;
	MACROBLOCK_ADDRESS_INCREMENT[0b100001001] = 12;
	MACROBLOCK_ADDRESS_INCREMENT[0b100001000] = 13;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000111] = 14;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000110] = 15;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000010111] = 16;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000010110] = 17;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000010101] = 18;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000010100] = 19;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000010011] = 20;
	MACROBLOCK_ADDRESS_INCREMENT[0b10000010010] = 21;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000100011] = 22;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000100010] = 23;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000100001] = 24;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000100000] = 25;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011111] = 26;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011110] = 27;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011101] = 28;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011100] = 29;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011011] = 30;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011010] = 31;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011001] = 32;
	MACROBLOCK_ADDRESS_INCREMENT[0b100000011000] = 33;
}

void HuffmanTable::init_MACROBLOCK_TYPE()
{
	for(int i=0;i<8;i++)
	{
		MACROBLOCK_TYPE_I[i] = -17;
	}
	for(int i=0;i<128;i++)
	{
		MACROBLOCK_TYPE_P[i] = -17;
		MACROBLOCK_TYPE_B[i] = -17;
	}
	this->MACROBLOCK_TYPE[0] = NULL;
	this->MACROBLOCK_TYPE[1] = this->MACROBLOCK_TYPE_I;
	this->MACROBLOCK_TYPE[2] = this->MACROBLOCK_TYPE_P;
	this->MACROBLOCK_TYPE[3] = this->MACROBLOCK_TYPE_B;

	MACROBLOCK_TYPE_I[0b11] = 1;
	MACROBLOCK_TYPE_I[0b101] = 0b10001;

	MACROBLOCK_TYPE_P[0b11] = 0b01010;
	MACROBLOCK_TYPE_P[0b101] = 0b00010;
	MACROBLOCK_TYPE_P[0b1001] = 0b01000;
	MACROBLOCK_TYPE_P[0b100011] = 0b00001;
	MACROBLOCK_TYPE_P[0b100010] = 0b11010;
	MACROBLOCK_TYPE_P[0b100001] = 0b10010;
	MACROBLOCK_TYPE_P[0b1000001] = 0b10001;

	MACROBLOCK_TYPE_B[0b110] = 0b01100;
	MACROBLOCK_TYPE_B[0b111] = 0b01110;
	MACROBLOCK_TYPE_B[0b1010] = 0b00100;
	MACROBLOCK_TYPE_B[0b1011] = 0b00110;
	MACROBLOCK_TYPE_B[0b10010] = 0b01000;
	MACROBLOCK_TYPE_B[0b10011] = 0b01010;
	MACROBLOCK_TYPE_B[0b100011] = 0b00001;
	MACROBLOCK_TYPE_B[0b100010] = 0b11110;
	MACROBLOCK_TYPE_B[0b1000011] = 0b11010;
	MACROBLOCK_TYPE_B[0b1000010] = 0b10110;
	MACROBLOCK_TYPE_B[0b1000001] = 0b10001;
}

void HuffmanTable::init_MOTION_VECTOR()
{
	for(int i=0;i<4096;i++)
	{
		MOTION_VECTOR[i] = -17;
	}
	MOTION_VECTOR[0b100000011001] = -16;
	MOTION_VECTOR[0b100000011011] = -15;
	MOTION_VECTOR[0b100000011101] = -14;
	MOTION_VECTOR[0b100000011111] = -13;
	MOTION_VECTOR[0b100000100001] = -12;
	MOTION_VECTOR[0b100000100011] = -11;
	MOTION_VECTOR[0b10000010011] = -10;
	MOTION_VECTOR[0b10000010101] = -9;
	MOTION_VECTOR[0b10000010111] = -8;
	MOTION_VECTOR[0b100000111] = -7;
	MOTION_VECTOR[0b100001001] = -6;
	MOTION_VECTOR[0b100001011] = -5;
	MOTION_VECTOR[0b10000111] = -4;
	MOTION_VECTOR[0b100011] = -3;
	MOTION_VECTOR[0b10011] = -2;
	MOTION_VECTOR[0b1011] = -1;
	MOTION_VECTOR[0b11] = 0;
	MOTION_VECTOR[0b1010] = 1;
	MOTION_VECTOR[0b10010] = 2;
	MOTION_VECTOR[0b100010] = 3;
	MOTION_VECTOR[0b10000110] = 4;
	MOTION_VECTOR[0b100001010] = 5;
	MOTION_VECTOR[0b100001000] = 6;
	MOTION_VECTOR[0b100000110] = 7;
	MOTION_VECTOR[0b10000010110] = 8;
	MOTION_VECTOR[0b10000010100] = 9;
	MOTION_VECTOR[0b10000010010] = 10;
	MOTION_VECTOR[0b100000100010] = 11;
	MOTION_VECTOR[0b100000100000] = 12;
	MOTION_VECTOR[0b100000011110] = 13;
	MOTION_VECTOR[0b100000011100] = 14;
	MOTION_VECTOR[0b100000011010] = 15;
	MOTION_VECTOR[0b100000011000] = 16;
}

void HuffmanTable::init_MACROBLOCK_PATTERN()
{
	for(int i=0;i<1024;i++)
	{
		MACROBLOCK_PATTERN[i] = -17;
	}
	MACROBLOCK_PATTERN[0b1111] = 60;
	MACROBLOCK_PATTERN[0b11101] = 4;
	MACROBLOCK_PATTERN[0b11100] = 8;
	MACROBLOCK_PATTERN[0b11011] = 16;
	MACROBLOCK_PATTERN[0b11010] = 32;
	MACROBLOCK_PATTERN[0b110011] = 12;
	MACROBLOCK_PATTERN[0b110010] = 48;
	MACROBLOCK_PATTERN[0b110001] = 20;
	MACROBLOCK_PATTERN[0b110000] = 40;
	MACROBLOCK_PATTERN[0b101111] = 28;
	MACROBLOCK_PATTERN[0b101110] = 44;
	MACROBLOCK_PATTERN[0b101101] = 52;
	MACROBLOCK_PATTERN[0b101100] = 56;
	MACROBLOCK_PATTERN[0b101011] = 1;
	MACROBLOCK_PATTERN[0b101010] = 61;
	MACROBLOCK_PATTERN[0b101001] = 2;
	MACROBLOCK_PATTERN[0b101000] = 62;
	MACROBLOCK_PATTERN[0b1001111] = 24;
	MACROBLOCK_PATTERN[0b1001110] = 36;
	MACROBLOCK_PATTERN[0b1001101] = 3;
	MACROBLOCK_PATTERN[0b1001100] = 63;
	MACROBLOCK_PATTERN[0b10010111] = 5;
	MACROBLOCK_PATTERN[0b10010110] = 9;
	MACROBLOCK_PATTERN[0b10010101] = 17;
	MACROBLOCK_PATTERN[0b10010100] = 33;
	MACROBLOCK_PATTERN[0b10010011] = 6;
	MACROBLOCK_PATTERN[0b10010010] = 10;
	MACROBLOCK_PATTERN[0b10010001] = 18;
	MACROBLOCK_PATTERN[0b10010000] = 34;
	MACROBLOCK_PATTERN[0b100011111] = 7;
	MACROBLOCK_PATTERN[0b100011110] = 11;
	MACROBLOCK_PATTERN[0b100011101] = 19;
	MACROBLOCK_PATTERN[0b100011100] = 35;
	MACROBLOCK_PATTERN[0b100011011] = 13;
	MACROBLOCK_PATTERN[0b100011010] = 49;
	MACROBLOCK_PATTERN[0b100011001] = 21;
	MACROBLOCK_PATTERN[0b100011000] = 41;
	MACROBLOCK_PATTERN[0b100010111] = 14;
	MACROBLOCK_PATTERN[0b100010110] = 50;
	MACROBLOCK_PATTERN[0b100010101] = 22;
	MACROBLOCK_PATTERN[0b100010100] = 42;
	MACROBLOCK_PATTERN[0b100010011] = 15;
	MACROBLOCK_PATTERN[0b100010010] = 51;
	MACROBLOCK_PATTERN[0b100010001] = 23;
	MACROBLOCK_PATTERN[0b100010000] = 43;
	MACROBLOCK_PATTERN[0b100001111] = 25;
	MACROBLOCK_PATTERN[0b100001110] = 37;
	MACROBLOCK_PATTERN[0b100001101] = 26;
	MACROBLOCK_PATTERN[0b100001100] = 38;
	MACROBLOCK_PATTERN[0b100001011] = 29;
	MACROBLOCK_PATTERN[0b100001010] = 45;
	MACROBLOCK_PATTERN[0b100001001] = 53;
	MACROBLOCK_PATTERN[0b100001000] = 57;
	MACROBLOCK_PATTERN[0b100000111] = 30;
	MACROBLOCK_PATTERN[0b100000110] = 46;
	MACROBLOCK_PATTERN[0b100000101] = 54;
	MACROBLOCK_PATTERN[0b100000100] = 58;
	MACROBLOCK_PATTERN[0b1000000111] = 31;
	MACROBLOCK_PATTERN[0b1000000110] = 47;
	MACROBLOCK_PATTERN[0b1000000101] = 55;
	MACROBLOCK_PATTERN[0b1000000100] = 59;
	MACROBLOCK_PATTERN[0b1000000011] = 27;
	MACROBLOCK_PATTERN[0b1000000010] = 39;
}

void HuffmanTable::init_DCT_COEFFIECIENTS()
{
	for(int i=0;i<512;i++)
	{
		if(i<256)
			DCT_DC_SIZE_LUMINANCE[i] = -17;
		DCT_DC_SIZE_CHROMINANCE[i] = -17;
	}
	DCT_DC_SIZE_LUMINANCE[0b1100] = 0;
	DCT_DC_SIZE_LUMINANCE[0b100] = 1;
	DCT_DC_SIZE_LUMINANCE[0b101] = 2;
	DCT_DC_SIZE_LUMINANCE[0b1101] = 3;
	DCT_DC_SIZE_LUMINANCE[0b1110] = 4;
	DCT_DC_SIZE_LUMINANCE[0b11110] = 5;
	DCT_DC_SIZE_LUMINANCE[0b111110] = 6;
	DCT_DC_SIZE_LUMINANCE[0b1111110] = 7;
	DCT_DC_SIZE_LUMINANCE[0b11111110] = 8;

	DCT_DC_SIZE_CHROMINANCE[0b100] = 0;
	DCT_DC_SIZE_CHROMINANCE[0b101] = 1;
	DCT_DC_SIZE_CHROMINANCE[0b110] = 2;
	DCT_DC_SIZE_CHROMINANCE[0b1110] = 3;
	DCT_DC_SIZE_CHROMINANCE[0b11110] = 4;
	DCT_DC_SIZE_CHROMINANCE[0b111110] = 5;
	DCT_DC_SIZE_CHROMINANCE[0b1111110] = 6;
	DCT_DC_SIZE_CHROMINANCE[0b11111110] = 7;
	DCT_DC_SIZE_CHROMINANCE[0b111111110] = 8;
}


void HuffmanTable::init_VARIABLE_LENGTH_CODE(){
	for(int i=0;i<262144;i++)
	{
		DCT_COEFF[i] = -17;
	}
	for(int lst = 0; lst<2; lst++){
		DCT_COEFF[0b10110 | lst] = ( 1 << 8 | 1);
		DCT_COEFF[0b101000 | lst] = ( 0 << 8 | 2);
		DCT_COEFF[0b101010 | lst] = ( 2 << 8 | 1);
		DCT_COEFF[0b1001010 | lst] = ( 0 << 8 | 3);
		DCT_COEFF[0b1001110 | lst] = ( 3 << 8 | 1);
		DCT_COEFF[0b1001100 | lst] = ( 4 << 8 | 1);
		DCT_COEFF[0b10001100 | lst] = ( 1 << 8 | 2);
		DCT_COEFF[0b10001110 | lst] = ( 5 << 8 | 1);
		DCT_COEFF[0b10001010 | lst] = ( 6 << 8 | 1);
		DCT_COEFF[0b10001000 | lst] = ( 7 << 8 | 1);
		DCT_COEFF[0b100001100 | lst] = ( 0 << 8 | 4);
		DCT_COEFF[0b100001000 | lst] = ( 2 << 8 | 2);
		DCT_COEFF[0b100001110 | lst] = ( 8 << 8 | 1);
		DCT_COEFF[0b100001010 | lst] = ( 9 << 8 | 1);
		DCT_COEFF[0b1001001100 | lst] = ( 0 << 8 | 5);
		DCT_COEFF[0b1001000010 | lst] = ( 0 << 8 | 6);
		DCT_COEFF[0b1001001010 | lst] = ( 1 << 8 | 3);
		DCT_COEFF[0b1001001000 | lst] = ( 3 << 8 | 2);
		DCT_COEFF[0b1001001110 | lst] = ( 10 << 8 | 1);
		DCT_COEFF[0b1001000110 | lst] = ( 11 << 8 | 1);
		DCT_COEFF[0b1001000100 | lst] = ( 12 << 8 | 1);
		DCT_COEFF[0b1001000000 | lst] = ( 13 << 8 | 1);
		DCT_COEFF[0b100000010100 | lst] = ( 0 << 8 | 7);
		DCT_COEFF[0b100000011000 | lst] = ( 1 << 8 | 4);
		DCT_COEFF[0b100000010110 | lst] = ( 2 << 8 | 3);
		DCT_COEFF[0b100000011110 | lst] = ( 4 << 8 | 2);
		DCT_COEFF[0b100000010010 | lst] = ( 5 << 8 | 2);
		DCT_COEFF[0b100000011100 | lst] = ( 14 << 8 | 1);
		DCT_COEFF[0b100000011010 | lst] = ( 15 << 8 | 1);
		DCT_COEFF[0b100000010000 | lst] = ( 16 << 8 | 1);
		DCT_COEFF[0b10000000111010 | lst] = ( 0 << 8 | 8);
		DCT_COEFF[0b10000000110000 | lst] = ( 0 << 8 | 9);
		DCT_COEFF[0b10000000100110 | lst] = ( 0 << 8 | 10);
		DCT_COEFF[0b10000000100000 | lst] = ( 0 << 8 | 11);
		DCT_COEFF[0b10000000110110 | lst] = ( 1 << 8 | 5);
		DCT_COEFF[0b10000000101000 | lst] = ( 2 << 8 | 4);
		DCT_COEFF[0b10000000111000 | lst] = ( 3 << 8 | 3);
		DCT_COEFF[0b10000000100100 | lst] = ( 4 << 8 | 3);
		DCT_COEFF[0b10000000111100 | lst] = ( 6 << 8 | 2);
		DCT_COEFF[0b10000000101010 | lst] = ( 7 << 8 | 2);
		DCT_COEFF[0b10000000100010 | lst] = ( 8 << 8 | 2);
		DCT_COEFF[0b10000000111110 | lst] = ( 17 << 8 | 1);
		DCT_COEFF[0b10000000110100 | lst] = ( 18 << 8 | 1);
		DCT_COEFF[0b10000000110010 | lst] = ( 19 << 8 | 1);
		DCT_COEFF[0b10000000101110 | lst] = ( 20 << 8 | 1);
		DCT_COEFF[0b10000000101100 | lst] = ( 21 << 8 | 1);
		DCT_COEFF[0b100000000110100 | lst] = ( 0 << 8 | 12);
		DCT_COEFF[0b100000000110010 | lst] = ( 0 << 8 | 13);
		DCT_COEFF[0b100000000110000 | lst] = ( 0 << 8 | 14);
		DCT_COEFF[0b100000000101110 | lst] = ( 0 << 8 | 15);
		DCT_COEFF[0b100000000101100 | lst] = ( 1 << 8 | 6);
		DCT_COEFF[0b100000000101010 | lst] = ( 1 << 8 | 7);
		DCT_COEFF[0b100000000101000 | lst] = ( 2 << 8 | 5);
		DCT_COEFF[0b100000000100110 | lst] = ( 3 << 8 | 4);
		DCT_COEFF[0b100000000100100 | lst] = ( 5 << 8 | 3);
		DCT_COEFF[0b100000000100010 | lst] = ( 9 << 8 | 2);
		DCT_COEFF[0b100000000100000 | lst] = ( 10 << 8 | 2);
		DCT_COEFF[0b100000000111110 | lst] = ( 22 << 8 | 1);
		DCT_COEFF[0b100000000111100 | lst] = ( 23 << 8 | 1);
		DCT_COEFF[0b100000000111010 | lst] = ( 24 << 8 | 1);
		DCT_COEFF[0b100000000111000 | lst] = ( 25 << 8 | 1);
		DCT_COEFF[0b100000000110110 | lst] = ( 26 << 8 | 1);
		DCT_COEFF[0b1000000000111110 | lst] = ( 0 << 8 | 16);
		DCT_COEFF[0b1000000000111100 | lst] = ( 0 << 8 | 17);
		DCT_COEFF[0b1000000000111010 | lst] = ( 0 << 8 | 18);
		DCT_COEFF[0b1000000000111000 | lst] = ( 0 << 8 | 19);
		DCT_COEFF[0b1000000000110110 | lst] = ( 0 << 8 | 20);
		DCT_COEFF[0b1000000000110100 | lst] = ( 0 << 8 | 21);
		DCT_COEFF[0b1000000000110010 | lst] = ( 0 << 8 | 22);
		DCT_COEFF[0b1000000000110000 | lst] = ( 0 << 8 | 23);
		DCT_COEFF[0b1000000000101110 | lst] = ( 0 << 8 | 24);
		DCT_COEFF[0b1000000000101100 | lst] = ( 0 << 8 | 25);
		DCT_COEFF[0b1000000000101010 | lst] = ( 0 << 8 | 26);
		DCT_COEFF[0b1000000000101000 | lst] = ( 0 << 8 | 27);
		DCT_COEFF[0b1000000000100110 | lst] = ( 0 << 8 | 28);
		DCT_COEFF[0b1000000000100100 | lst] = ( 0 << 8 | 29);
		DCT_COEFF[0b1000000000100010 | lst] = ( 0 << 8 | 30);
		DCT_COEFF[0b1000000000100000 | lst] = ( 0 << 8 | 31);
		DCT_COEFF[0b10000000000110000 | lst] = ( 0 << 8 | 32);
		DCT_COEFF[0b10000000000101110 | lst] = ( 0 << 8 | 33);
		DCT_COEFF[0b10000000000101100 | lst] = ( 0 << 8 | 34);
		DCT_COEFF[0b10000000000101010 | lst] = ( 0 << 8 | 35);
		DCT_COEFF[0b10000000000101000 | lst] = ( 0 << 8 | 36);
		DCT_COEFF[0b10000000000100110 | lst] = ( 0 << 8 | 37);
		DCT_COEFF[0b10000000000100100 | lst] = ( 0 << 8 | 38);
		DCT_COEFF[0b10000000000100010 | lst] = ( 0 << 8 | 39);
		DCT_COEFF[0b10000000000100000 | lst] = ( 0 << 8 | 40);
		DCT_COEFF[0b10000000000111110 | lst] = ( 1 << 8 | 8);
		DCT_COEFF[0b10000000000111100 | lst] = ( 1 << 8 | 9);
		DCT_COEFF[0b10000000000111010 | lst] = ( 1 << 8 | 10);
		DCT_COEFF[0b10000000000111000 | lst] = ( 1 << 8 | 11);
		DCT_COEFF[0b10000000000110110 | lst] = ( 1 << 8 | 12);
		DCT_COEFF[0b10000000000110100 | lst] = ( 1 << 8 | 13);
		DCT_COEFF[0b10000000000110010 | lst] = ( 1 << 8 | 14);
		DCT_COEFF[0b100000000000100110 | lst] = ( 1 << 8 | 15);
		DCT_COEFF[0b100000000000100100 | lst] = ( 1 << 8 | 16);
		DCT_COEFF[0b100000000000100010 | lst] = ( 1 << 8 | 17);
		DCT_COEFF[0b100000000000100000 | lst] = ( 1 << 8 | 18);
		DCT_COEFF[0b100000000000101000 | lst] = ( 6 << 8 | 3);
		DCT_COEFF[0b100000000000110100 | lst] = ( 11 << 8 | 2);
		DCT_COEFF[0b100000000000110010 | lst] = ( 12 << 8 | 2);
		DCT_COEFF[0b100000000000110000 | lst] = ( 13 << 8 | 2);
		DCT_COEFF[0b100000000000101110 | lst] = ( 14 << 8 | 2);
		DCT_COEFF[0b100000000000101100 | lst] = ( 15 << 8 | 2);
		DCT_COEFF[0b100000000000101010 | lst] = ( 16 << 8 | 2);
		DCT_COEFF[0b100000000000111110 | lst] = ( 27 << 8 | 1);
		DCT_COEFF[0b100000000000111100 | lst] = ( 28 << 8 | 1);
		DCT_COEFF[0b100000000000111010 | lst] = ( 29 << 8 | 1);
		DCT_COEFF[0b100000000000111000 | lst] = ( 30 << 8 | 1);
		DCT_COEFF[0b100000000000110110 | lst] = ( 31 << 8 | 1);
	}
}