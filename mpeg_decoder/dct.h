#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdbool.h>
#include <algorithm>
#include <cstring>

#define _Pi 	3.14159265358979323846
#define sqrt2 1.414213562373095
#define _N 8

class DCT
{
public:
	DCT();
	void initial_DCT_table(void);
	void IDCT2_8X8(int *block);
private:
	double** cosine;
	double alpha(int i);
};
