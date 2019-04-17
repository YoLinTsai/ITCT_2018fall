#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jfif.h"
#include "time.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

void rgb2bmp(int *rgb, Mat& bmp, int width, int height)
{
	int *r = &rgb[0];
	int *g = &rgb[height * width];
	int *b = &rgb[2 * height * width];
	int i,j;
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			int temp_r,temp_g,temp_b;
			if(*r<0)
				temp_r = 0;
			else if(*r>=255)
				temp_r = 255;
			else
				temp_r = *r;

			if(*g<0)
				temp_g = 0;
			else if(*g>=255)
				temp_g = 255;
			else
				temp_g = *g;

			if(*b<0)
				temp_b = 0;
			else if(*b>=255)
				temp_b = 255;
			else
				temp_b = *b;

			bmp.at<Vec3b>(i,j)[0] = temp_b;
			bmp.at<Vec3b>(i,j)[1] = temp_g;
			bmp.at<Vec3b>(i,j)[2] = temp_r;
			r += 1;
			g += 1;
			b += 1;
		}
	}
	return;
}

char *changefilename(char jpgfilename[])
{
	char * pch;
	pch = strtok (jpgfilename,".");
	strcat(pch, ".bmp");
	return pch;
}

int main(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("WRONG INPUT ARGUMENT!: ./main input_file(encoded_jpg)\n");
		exit(0);
	}

	printf("\tStart to decode %s to bmp file\n",argv[1]);
	clock_t launch = clock();
	FILE *fp = NULL;
	JFIF *jfif = NULL;
	int *rgb = NULL;
	long fileSize;
	unsigned char *dataBuffer;

	fp = fopen(argv[1], "rb");
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET); //back to beginning offset
	//printf("%d\n",fileSize);
	dataBuffer = (unsigned char *)malloc((fileSize + 1)*sizeof(unsigned char));
	fread(dataBuffer, fileSize, 1, fp);
	jfif = jfif_load(dataBuffer, fileSize);
	Mat bmp(jfif->height, jfif->width, CV_8UC3);
	rgb = jfif_decode(jfif);
	rgb2bmp(rgb, bmp, jfif->width, jfif->height);
	printf("\tTotal elapsed time is: %f seconds.\n",(double)(clock() - launch) / 1000);
	char *outputFile = changefilename(argv[1]);
	printf("\tSave bmp file as %s\n", outputFile);
	imwrite(outputFile, bmp);
	//done
	return 0;
}