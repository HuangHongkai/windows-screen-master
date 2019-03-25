#include "ScreenMaster.h"



ScreenMaster::ScreenMaster()
{
}


ScreenMaster::ScreenMaster(create_t * conf)
{
	quit = false;
	running = false;
	sleep_msec = conf->sleep_msec;
	frame = conf->frame;
	display_change = conf->display_change;

	hthread = CreateThread(NULL, 0, __loop, this, 0, &tid);
	if (!hthread)
	{
		CloseHandle(hthread);
		printf("fatal error\n");
		exit(-1);
	}
	SetThreadPriority(hthread, THREAD_PRIORITY_HIGHEST);

}


ScreenMaster::~ScreenMaster()
{
}

DWORD __stdcall ScreenMaster::__loop(LPVOID instance)
{
	ScreenMaster* master = (ScreenMaster*)instance;
	if (__init_gdi(instance))
	{

	}
	return 0;
}

DWORD ScreenMaster::__init_gdi(LPVOID instance)
{
	DEVMODE devmode;
	ScreenMaster* master = (ScreenMaster*)instance;
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL bRet = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	if (!bRet)
		return false;
	master->gdi.cx = devmode.dmPelsWidth;
	master->gdi.cy = devmode.dmPelsHeight;
	master->gdi.bitcount = devmode.dmBitsPerPel;
	if (master->gdi.bitcount != 8 && master->gdi.bitcount != 16 && master->gdi.bitcount != 32 && master->gdi.bitcount != 24)
		return false;
	master->gdi.line_bytes = master->gdi.cx * master->gdi.bitcount / 8;
	master->gdi.line_stride = (master->gdi.line_bytes + 3) / 4 * 4;

	return 0;
}
