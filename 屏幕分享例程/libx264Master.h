#pragma once
#define    WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include "ffmpeg.h"
#include <x264.h>

#pragma comment(lib, "libx264.lib") //¾²Ì¬¿â

class libx264Master
{
private:
	int width, height;
	FILE* fp_dst;
	int frame_num;
	int csp;
	int iNal;
	char filename[100];
	int y_size;
	x264_nal_t* pNals;
	x264_t* pHandle;
	x264_picture_t* pPic_in;
	x264_picture_t* pPic_out;
	x264_param_t* pParam;

	void init();
	
public:
	libx264Master(int width, int height, const char* filepath);
	~libx264Master();
	bool encode(AVFrame* pFrameYUV);
	bool flush();
};

