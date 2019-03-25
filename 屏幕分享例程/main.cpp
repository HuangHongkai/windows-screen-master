#define    WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include "rgb2yuv.h"
#include "SDLMaster.h"
#include "MonitorMaster.h"
#include "ffmpegEncoder.h"
#include "libx264Master.h"

using namespace std;

#pragma comment(lib ,"ws2_32.lib")

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
extern "C" _CRT_STDIO_INLINE int __CRTDECL fprintf(
	_Inout_                       FILE*       const _Stream,
	_In_z_ _Printf_format_string_ char const* const _Format,
	...);

struct cap_screen_t
{
	HDC memdc;
	HBITMAP hbmp;
	unsigned char* buffer;
	int            length;

	int width;
	int height;
	int bitcount;
	int left, top;
};

extern MonitorMaster::VEC_MONITOR_INFO vecMonitorListInfo;

int init_cap_screen(struct cap_screen_t* sc, int indexOfMonitor = 0)
{
	MonitorMaster::GetAllMonitorInfo();
	if (indexOfMonitor >= vecMonitorListInfo.size())
		indexOfMonitor = 0;
	DEVMODE devmode;
	BOOL bRet;
	BITMAPINFOHEADER bi;
	sc->width = vecMonitorListInfo[indexOfMonitor]->nWidth;
	sc->height = vecMonitorListInfo[indexOfMonitor]->nHeight;
	sc->bitcount = vecMonitorListInfo[indexOfMonitor]->nBits;
	sc->left = vecMonitorListInfo[indexOfMonitor]->area.left;
	sc->top = vecMonitorListInfo[indexOfMonitor]->area.top;
	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = sc->width;
	bi.biHeight = -sc->height; //从上朝下扫描
	bi.biPlanes = 1;
	bi.biBitCount = sc->bitcount; //RGB
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	HDC hdc = GetDC(NULL); //屏幕DC
	sc->memdc = CreateCompatibleDC(hdc);
	sc->buffer = NULL;
	sc->hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&sc->buffer, NULL, 0);
	ReleaseDC(NULL, hdc);
	SelectObject(sc->memdc, sc->hbmp); ///
	sc->length = sc->height* (((sc->width*sc->bitcount / 8) + 3) / 4 * 4);
	return 0;
}

HCURSOR FetchCursorHandle()
{
	CURSORINFO hCur;
	ZeroMemory(&hCur, sizeof(hCur));
	hCur.cbSize = sizeof(hCur);
	GetCursorInfo(&hCur);
	return hCur.hCursor;
}

void AddCursor(HDC hMemDC, POINT origin)
{

	POINT xPoint;
	GetCursorPos(&xPoint);
	xPoint.x -= origin.x;
	xPoint.y -= origin.y;
	if (xPoint.x < 0 || xPoint.y < 0)
		return;
	HCURSOR hcur = FetchCursorHandle();
	ICONINFO iconinfo;
	BOOL ret;
	ret = GetIconInfo(hcur, &iconinfo);
	if (ret)
	{
		xPoint.x -= iconinfo.xHotspot;
		xPoint.y -= iconinfo.yHotspot;
		if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);
		if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
	}
	DrawIcon(hMemDC, xPoint.x, xPoint.y, hcur);
}

int blt_cap_screen(struct cap_screen_t* sc)
{
	HDC hdc = GetDC(NULL);
	BitBlt(sc->memdc, 0, 0, sc->width, sc->height, hdc, sc->left, sc->top, SRCCOPY); // 截屏
	AddCursor(sc->memdc, POINT{ sc->left, sc->top }); // 增加鼠标进去
	ReleaseDC(NULL, hdc);
	return 0;
}
#include "web_stream.h"

int main(int argc, char* argv[])
{
	struct cap_screen_t sc;
	BYTE* out;
	AVFrame* frame;
	web_stream* web = new web_stream;
	time_t start, end;
	WSADATA d;

	ffmpeg_init();
	init_cap_screen(&sc, 1);  // 1代表第二块显示器，0代表第一块显示器，依次类推，如果显示器数量不足则为0
	out = (BYTE*)malloc(sc.length);

	// SDLMaster::init(sc.width, sc.height); // SDL播放器，可以用于播放测试
	YUVencoder enc(sc.width, sc.height);

	// 以下为3个不同的编码器，任选一个即可
	x264Encoder ffmpeg264(sc.width, sc.height,  "save.h264"); // save.h264为本地录屏文件 可以使用vlc播放器播放
	//mpeg1Encoder ffmpegmpeg1(sc.width, sc.height, "save.mpg");
	//libx264Master libx264(sc.width, sc.height, "save2.h264");
	
	WSAStartup(0x0202, &d);
	web->start("0.0.0.0", 8000); // 8000端口侦听
	time(&start);
	for (int i = 0; ; i++)
	{
		blt_cap_screen(&sc);
		char* buf = NULL;
		dp_frame_t a;
		a.bitcount = 32;
		a.buffer = (char*)sc.buffer;
		a.line_bytes = sc.width * sc.bitcount / 8;
		a.line_stride = (a.line_bytes + 3) / 4 * 4;
		a.cx = sc.width;
		a.cy = sc.height;
		web->frame(&a);  // 发送帧

		frame = enc.encode(sc.buffer, sc.length, sc.width, sc.height, out, sc.length);
		
		if (frame == NULL)
		{
			printf("Encoder error!!\n"); Sleep(1000); continue;
		}

		// 一下是3种编码方式，任选一种均可
		ffmpeg264.encode(frame);
		// ffmpegmpeg1.encode(frame); 
		// libx264.encode(frame);

		// SDL播放器播放视频，取消注释即可播放
		// SDLMaster::updateScreen(frame); 

		free(buf);
	}
	time(&end);
	printf("%f\n", difftime(end, start));
	//ffmpeg264.flush(frame, &sender);
	SDL_Quit();
	return 0;
}
