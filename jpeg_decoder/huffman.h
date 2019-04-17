#pragma once
#include "stdio.h"
#include "stdlib.h"
#include <string>
#include <stdbool.h>

void store_huffmancode(int idx,std::string code,int value);
void traversal_huffmantable(int idx);
int find_HufCode(std::string code, int DCAC, int table_idx);
int check_HufCode(std::string code, int DCAC, int table_idx);
int getValue(std::string code, int codeLength);