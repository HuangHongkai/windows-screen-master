#include <atlstr.h>
#include <vector>
#include <WinDef.h>
#include <tchar.h>

namespace MonitorMaster
{
	
	// 显示器信息
	struct MonitorInfo
	{
		WCHAR       szDevice[CCHDEVICENAME];				// 显示器名称
		RECT area; // 显示器矩形区域
		DWORD nWidth, nHeight, nFreq, nBits;
	};

	typedef std::vector<MonitorInfo*> VEC_MONITOR_INFO;  // 所有的显示器信息	
	

	// 枚举显示器回调函数
	BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,
		HDC hdc,
		LPRECT lpRMonitor,
		LPARAM dwData);

	// 得到所有显示器的信息
	void GetAllMonitorInfo();

	//得到屏幕当前分辨率
	void GetCurrentReselotion(int& nWidth, int& nHeight, int& nFreq, int& nBits);

	//根据屏幕ID取获取屏幕的对应分辨率
	void GetCurrentReselotion(LPCWSTR lpszDeviceName, int& nWidth, int& nHeight, int& nFreq, int& nBits);

	int EnumMonitorInfo();

	//修改分辨率
	int ChangMonitorReselotion(HMONITOR hMonitor, const int nWidth, const int nHight, const int nFre, const int nColorBits);
};