#pragma once
#include <Windows.h>
#include <stdio.h>
#include <dxgi1_6.h>

struct screen_cap_conf
{

};

struct rect_t
{
	RECT   rc;             ///发生变化的矩形框
	/////
	char*  line_buffer;    ///矩形框数据起始地址
	int    line_bytes;     ///每行（矩形框width对应）的数据长度
	int    line_nextpos;   ///从0开始，第N行的数据地址: line_buffer + N*line_nextpos 。
	int    line_count;     ///等于矩形框高度 height
};

struct frame_t
{
	int        cx;          ///屏幕宽度
	int        cy;          ///屏幕高度
	int        line_bytes;  ///每个扫描行的实际数据长度
	int        line_stride; ///每个扫描行的4字节对齐的数据长度
	int        bitcount;    ///8.16.24.32 位深度, 8位是256调色板； 16位是555格式的图像

	int        length;      ///屏幕数据长度 line_stride*cy
	char*      buffer;      ///屏幕数据
	/////
	int        rc_count;    ///变化区域个数
	rect_t*   rc_array;    ///变化区域

	
	LPVOID      web;   
};

typedef int (*DISPLAYCHANGE_CALLBACK)(int width, int height, int bitcount, LPVOID params);
typedef int(*FRAME_CALLBACK)(frame_t frame);

struct create_t
{
	// 初始化ScreenMaster结构体
	DISPLAYCHANGE_CALLBACK display_change;
	FRAME_CALLBACK frame;
	LPVOID web;
	ULONG sleep_msec;
};

struct gdi_cap_t
{
	int        cx;
	int        cy;
	int        line_bytes;
	int        line_stride;
	int        bitcount; ////
	HDC        memdc;
	HBITMAP    hbmp;
	
	byte*      buffer; ///
	byte*      back_buf; ///
};

class ScreenMaster
{
public:
	ULONG sleep_msec; // 毫秒
	bool quit;
	bool running;
	screen_cap_conf cap_conf;
	DISPLAYCHANGE_CALLBACK display_change;
	FRAME_CALLBACK frame;
	HANDLE hthread;
	DWORD tid;
	gdi_cap_t gdi;

public:
	ScreenMaster();
	ScreenMaster(create_t *conf);
	~ScreenMaster();

private:
	static DWORD WINAPI __loop(LPVOID instance);
	static DWORD __init_gdi(LPVOID instance);
};

