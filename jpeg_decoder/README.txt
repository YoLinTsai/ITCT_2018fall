======================================
ffjpeg 使用說明 (2018/05/09)

 	         By Yu-Lin Tsai
======================================

Environment:

Thread model: win32
gcc version 4.8.1 (GCC)
Opencv Library
Windows 10

======================================

Compile command:

g++ -c jfif.cpp
g++ -c zigzag.cpp
g++ -c huffman.cpp
g++ -c dct.cpp
g++ -I"C:\opencv-build\install\include" -L"C:\opencv-build\install\x86\mingw\lib" -c main.cpp -lopencv_core310 -lopencv_highgui310 -lopencv_imgcodecs310 -lopencv_imgproc310 
g++ -I"C:\opencv-build\install\include" -L"C:\opencv-build\install\x86\mingw\lib" jfif.o main.o huffman.o zigzag.o dct.o -lopencv_core310 -lopencv_highgui310 -lopencv_imgcodecs310 -lopencv_imgproc310  -o main

因為我是在command中將opencv library連結到程式，如果本機有將opencv裝好應該可以直接透過g++ compile。

======================================

How to run?

output bmp file
	./main [input_file]

======================================

此外，opencv library我只使用了create/save bmp的功能。