/// by fanxiushu 2017-05-14

#pragma once

struct dp_frame_t
{
	int        cx;          ///屏幕宽度
	int        cy;          ///屏幕高度
	int        line_bytes;  ///每个扫描行的实际数据长度
	int        line_stride; ///每个扫描行的4字节对齐的数据长度
	int        bitcount;    ///8.16.24.32 位深度, 8位是256调色板； 16位是555格式的图像

	char*      buffer;      ///屏幕数据
};


class web_stream
{
protected:
	bool quit;
	int listenfd;
	vector<int> socks;
	CRITICAL_SECTION cs;
	static DWORD CALLBACK accept_thread(void* _p);
	static DWORD CALLBACK client_thread(void* _p);
	///
	int jpeg_quality;
public:
	web_stream();
	~web_stream();
	
	////
	int start(const char* strip, int port);
	void set_jpeg_quality(int quality) { jpeg_quality = quality; }
	void frame(struct dp_frame_t* frame);
};

