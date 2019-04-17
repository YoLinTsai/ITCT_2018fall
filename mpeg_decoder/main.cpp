#include<iostream>
#include<bits/stdc++.h>
#include <stdio.h>

#include"decode.h"

using namespace std;

int main(int argc, char** argv)
{
	if ( argc != 2 )
    {
        printf("usage error:./mpegDecoder video_filename\n");
        exit(0);
    }
    MPEG1 mpeg(argv[1]);
    mpeg.decode();
    return 0;
}