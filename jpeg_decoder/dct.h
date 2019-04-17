#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdbool.h>
#include <algorithm>
#include <cstring>

#define sqrt2 1.414213562373095
#define N 8

void initial_DCT_table(void);
void IDCT2_8X8(int *block);