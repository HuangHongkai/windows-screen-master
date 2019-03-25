#include "MonitorMaster.h"
#include <iostream>


std::vector<HMONITOR> g_hMonitorGroup;
MonitorMaster::VEC_MONITOR_INFO vecMonitorListInfo;

BOOL CALLBACK MonitorMaster::MonitorEnumProc(HMONITOR hMonitor,
	HDC hdc,
	LPRECT lpRMonitor,
	LPARAM dwData)
{
	g_hMonitorGroup.push_back(hMonitor);
	MonitorInfo* monitorinfo = new MonitorInfo();
	monitorinfo->area.left = lpRMonitor->left;
	monitorinfo->area.right = lpRMonitor->right;
	monitorinfo->area.top = lpRMonitor->top;
	monitorinfo->area.bottom = lpRMonitor->bottom;
	monitorinfo->nWidth = lpRMonitor->right - lpRMonitor->left;
	monitorinfo->nHeight = lpRMonitor->bottom - lpRMonitor->top;
	vecMonitorListInfo.push_back(monitorinfo);
	return 1;
}


void MonitorMaster::GetAllMonitorInfo()
{
	g_hMonitorGroup.clear();
	vecMonitorListInfo.clear();

	::EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	for (int i = 0; i < g_hMonitorGroup.size(); i++)
	{
		MONITORINFOEX mixTemp;
		memset(&mixTemp, 0, sizeof(MONITORINFOEX));
		mixTemp.cbSize = sizeof(MONITORINFOEX);

		GetMonitorInfo(g_hMonitorGroup[i], &mixTemp);
		wcscpy((*vecMonitorListInfo[i]).szDevice, mixTemp.szDevice);
		DEVMODE DeviceMode;
		EnumDisplaySettings(mixTemp.szDevice, ENUM_CURRENT_SETTINGS, &DeviceMode);
		(*vecMonitorListInfo[i]).nFreq = DeviceMode.dmDisplayFrequency;
		(*vecMonitorListInfo[i]).nBits = DeviceMode.dmBitsPerPel;
	}
}


int MonitorMaster::ChangMonitorReselotion(HMONITOR hMonitor, const int nWidth, const int nHight, const int nFre, const int nColorBits)
{
	if (NULL == hMonitor)
	{
		return -1;
	}
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);
	DEVMODE DeviceMode;
	ZeroMemory(&DeviceMode, sizeof(DEVMODE));
	DeviceMode.dmSize = sizeof(DEVMODE);

	BOOL bFlag = TRUE;
	bFlag = EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &DeviceMode);
	if (bFlag != TRUE)
	{
		return -1;
	}
	if (DeviceMode.dmPelsWidth == nWidth && DeviceMode.dmPelsHeight == nHight)
	{
		return 0;
	}
	DeviceMode.dmDisplayFlags = 0;
	DeviceMode.dmPelsWidth = nWidth;
	DeviceMode.dmPelsHeight = nHight;

	DeviceMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	int nRet = ChangeDisplaySettingsEx(mi.szDevice, &DeviceMode, NULL, CDS_GLOBAL | CDS_NORESET | CDS_UPDATEREGISTRY, NULL);
	if (DISP_CHANGE_BADMODE == nRet)
	{
		ChangeDisplaySettingsEx(mi.szDevice, &DeviceMode, NULL, CDS_GLOBAL | CDS_NORESET | CDS_UPDATEREGISTRY, NULL);

	}
	if (DISP_CHANGE_SUCCESSFUL == nRet)
	{
		return 0;
	}
	return -1;
}

void MonitorMaster::GetCurrentReselotion(int& nWidth, int& nHeight, int& nFreq, int& nBits)
{
	DEVMODE DeviceMode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DeviceMode);
	nWidth = DeviceMode.dmPelsWidth;
	nHeight = DeviceMode.dmPelsHeight;
	nFreq = DeviceMode.dmDisplayFrequency;
	nBits = DeviceMode.dmBitsPerPel;
}

void MonitorMaster::GetCurrentReselotion(LPCWSTR lpszDeviceName, int& nWidth, int& nHeight, int& nFreq, int& nBits)
{
	DEVMODE DeviceMode;
	EnumDisplaySettings(lpszDeviceName, ENUM_CURRENT_SETTINGS, &DeviceMode);
	nWidth = DeviceMode.dmPelsWidth;
	nHeight = DeviceMode.dmPelsHeight;
	nFreq = DeviceMode.dmDisplayFrequency;
	nBits = DeviceMode.dmBitsPerPel;
	printf("DeviceName: %ws", lpszDeviceName);
	std::cout << " , Width: " << nWidth << " , Height: " << nHeight << " , bits: " << nBits << " , freq: " << nFreq << std::endl;
}

int MonitorMaster::EnumMonitorInfo()
{
	GetAllMonitorInfo();

	for (auto itBeg = vecMonitorListInfo.begin(); itBeg != vecMonitorListInfo.end() ; itBeg++)
	{
		printf("DeviceName:%ws ", (*itBeg)->szDevice);
		std::cout << "  left: " << (*itBeg)->area.left << "  right: " << (*itBeg)->area.right << "  botttom: " << (*itBeg)->area.bottom << "  top: " << (*itBeg)->area.top << "  nFreq: " << (*itBeg)->nFreq << "  nBits: " << (*itBeg)->nBits << std::endl;
	}

	return 0;
}