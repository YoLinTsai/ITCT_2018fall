#pragma once
#include "stdio.h"
#include "stdlib.h"

#define SOF0	192		//c0
#define DHT		196		//c4
#define	SOI		216		//d8
#define	EOI		217		//d9
#define SOS		218		//da
#define DQT		219		//db
#define DRI		221		//dd
#define APP0	224		//e0
#define APP15	239		//ef
#define FF		255		//ff

typedef struct {
    int       width;
    int       height;

    struct {
    	int id;
    	int precision;
    	int table[64];
    } QT[4];

    int channelNum;
    struct {
        int id;
        int sampleRate_X;
        int sampleRate_Y;
        int QT_idx;
        int HT_AC_idx;
        int HT_DC_idx;
    } channel_info[4];

    int Ri;

    int encodedDataLen;
    char *encodedData;
} JFIF;

JFIF* jfif_load(unsigned char *jpeg_data, long fileSize);
int* jfif_decode(void *_jfif);