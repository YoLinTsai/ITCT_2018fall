#include "dct.h"
#include <math.h>
#include <cmath>

DCT::DCT()
{
	this->cosine = new double*[8];
	for(int i=0;i<8;i++)
	{
		this->cosine[i] = new double[8];
	}
	this->initial_DCT_table();
}

double DCT::alpha(int i)
{
    if (i == 0)
    {
    	return sqrt2 * 0.25;
    }
    else
    {
		return 0.5;
    }
}

void DCT::initial_DCT_table(void)
{
	for (int i = 0; i < 8; i++)
	{
	     for (int j = 0; j < 8; j++)
	     {
	         this->cosine[j][i] = cos(_Pi * j * (2 * i + 1) / (2 * _N));
	     }
	}
}


void DCT::IDCT2_8X8(int *block)
{
	int i, j, u, v;
	double A[64];

	for (i = 0;i<8;i++)
	{
	    for (j = 0;j<8;j++)
	    {
	        A[j * 8 + i] = 0;
	        for (u = 0;u<8;u++)
	        {
	        	for (v = 0;v<8;v++)
	        	{
					A[j * 8 + i] += this->alpha(u) * this->alpha(v) * (double)block[u * 8 + v] * cosine[u][j] * cosine[v][i];
	        	}
	        }
	    }
	}

	for(i = 0;i<64;i++)
	{
		block[i] = (int) A[i];
	}
	return;
}