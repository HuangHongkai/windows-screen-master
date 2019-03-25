#pragma once
#define    WIN32_LEAN_AND_MEAN 
#include "ffmpeg.h"
#include <Windows.h>

class IEncoder
{
protected:
	AVCodec *pCodec;
	AVCodecContext *pCodecCtx;
	FILE* fp_out;
	AVPacket pkt;
	char filename[256];
	int got_output;
	int width, height;

	virtual void init() = 0;

public:
	bool encode(AVFrame* frame); 
	bool flush(AVFrame* frame);
};

class x264Encoder: public IEncoder
{
protected:
	void init();
	void init_header();
public:
	x264Encoder(int widht, int height, const char* filename);
	~x264Encoder();
};


class mpeg1Encoder: public IEncoder
{
protected:
	void init();
public:
	mpeg1Encoder(int width, int height, const char* filename);
	~mpeg1Encoder();
};