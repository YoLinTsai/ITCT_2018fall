#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdbool.h>
#include <map>
#include <algorithm>
#include "huffman.h"

using namespace std;

int twoPowers[17] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
typedef map<string, int> HuffmanTable;
HuffmanTable HT[4]; //XY 	X:	0 DC Y:	0 IDX 
					//			1 AC	1 IDX

void store_huffmancode(int idx,string code,int value)
{
	HT[idx][code] = value;
	return;
}

void traversal_huffmantable(int idx)
{
	printf("idx:%d\n",idx);
	for( HuffmanTable::const_iterator it = HT[idx].begin(); it != HT[idx].end(); ++it )
    {
    	string key = it->first;
		int value = it->second;
		printf("code:%s, value:%d\n",key.c_str(),value);
    }
    printf("\n");
	return;
}

int find_HufCode(std::string code, int DCAC, int table_idx)
{
	int idx = DCAC * 2 + table_idx;
	return HT[idx][code];
}

int check_HufCode(std::string code, int DCAC, int table_idx)
{
	int idx = DCAC * 2 + table_idx;
	if ( HT[idx].find(code) == HT[idx].end() ) 
	{
	  return 0; //not found
	} else 
	{
	  return 1;	//found!
	}
}

int getValue(std::string code, int codeLength){
	if (codeLength)
	{
		int value = 0;
    	int midPoint = twoPowers[codeLength - 1];
    	int lowPoint = -twoPowers[codeLength] + 1;
		int mul = midPoint;
		for (int i = 0; i < codeLength; i++)
		{
			value += (int)(code[i]-'0') * mul;
			mul = mul >> 1;
		}
		if (value >= midPoint)
			return value;
		else
			return value + lowPoint;
	}
	else
		return 0;

}

