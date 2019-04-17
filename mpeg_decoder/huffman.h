#pragma once
#include"bitstream.h"

class HuffmanTable {

public:
	HuffmanTable();
	int* MACROBLOCK_ADDRESS_INCREMENT;
	int* MACROBLOCK_TYPE_I;
	int* MACROBLOCK_TYPE_P;
	int* MACROBLOCK_TYPE_B;
	int* MACROBLOCK_TYPE[4];
	int* MOTION_VECTOR;
	int* MACROBLOCK_PATTERN;
	int* DCT_DC_SIZE_LUMINANCE;
	int* DCT_DC_SIZE_CHROMINANCE;
	int* DCT_COEFF;
    int getValue(int* table, BITstream *bitstream);
    void get_DCT_COEFF_FIRST(int* table, BITstream *bitstream, int& run, int& level);
    void get_DCT_COEFF_NEXT(int* table, BITstream *bitstream, int& run, int& level);

private:
    void init_all_table();
    void init_MACROBLOCK_ADDRESS_INCREMENT();
    void init_MACROBLOCK_TYPE();
    void init_MOTION_VECTOR();
    void init_MACROBLOCK_PATTERN();
    void init_DCT_COEFFIECIENTS();
    void init_VARIABLE_LENGTH_CODE();
};