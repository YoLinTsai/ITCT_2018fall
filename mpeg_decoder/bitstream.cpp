#include "bitstream.h"

BITstream::BITstream(const char *filename)
{
	this->buffer = NULL;
	this->loadmpegfile(filename);
}

void BITstream::loadmpegfile(const char *filename)
{
	std::ifstream fp (filename, std::ios::in | std::ios::binary);
	fp.seekg (0, fp.end);
    this->fileSize = fp.tellg();
    fp.seekg (0, fp.beg);

	this->buffer = new char[this->fileSize];
	fp.read((char *)this->buffer,this->fileSize);
	printf("size:%d\n",this->fileSize);
	fp.close();
	this->bit_index = 0;
}

BITstream::~BITstream()
{

}

int BITstream::seek_bits(int size)
{
	int ret = this->read_bits(size);
	this->bit_index -= size;
	return ret;
}

int BITstream::read_bits(int size)
{
	int temp;
	int ret=0;
	for(int i=0;i<size;i++)
	{
		temp = this->get_a_bit();
		ret = ((ret<<1) | temp);
		//printf("%d",temp);
	}
	//printf("\n");
	return ret;
}

int BITstream::get_a_bit()
{
	int ret=0;
	int byte_index = this->bit_index / 8;
	int bit_index = this->bit_index % 8;
	ret = (buffer[byte_index] & (1<<(7-bit_index))) != 0;
	this->bit_index += 1;
	//printf("%d %d %d\n",byte_index,bit_index,ret);
	return ret;
}

void BITstream::skip(int size)
{
	this->bit_index += size;
	return;
}

void BITstream::align()
{
    this->bit_index = ((this->bit_index + 7) >> 3) << 3;
    return;
}