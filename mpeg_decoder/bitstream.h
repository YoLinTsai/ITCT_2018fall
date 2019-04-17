#pragma once
#include <fstream>
#include <stdlib.h>
class BITstream {
public:
	BITstream(const char *filename);
	~BITstream();
	void loadmpegfile(const char *filename);
	void skip(int size);
	int get_a_bit();
	int read_bits(int size);
	int seek_bits(int size);
	void align();
private:
	char* buffer;
	int bit_index;
	int fileSize;
};