
#include <Windows.h>

#include <string>
#include <vector>
#include <process.h>    /* _beginthread, _endthread */  

#include "../../lsMisc/HighDPI.h"
#include "C:/Linkout/CommonDLL/TimedMessageBox.h"
#include "../../lsMisc/RevealFolder.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/stdosd/stdosd.h"

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;



#define APPNAME L"OpenNetworkFolder"
#define APPVERSION L"ver1.0.1"

DWORD ShowTimedMessage(LPCTSTR pMessage)
{
	HMODULE hModule = LoadLibrary(L"TimedMessageBox.dll");
	if (!hModule)
	{
		MessageBox(NULL, L"Failed to load TimedMessageBox.dll", APPNAME, MB_ICONEXCLAMATION);
		return IDCANCEL;
	}
	FNTimedMessageBox2 func2 = NULL;
	func2 = (FNTimedMessageBox2)GetProcAddress(hModule, "fnTimedMessageBox2");
	if (!func2)
	{
		MessageBox(NULL, L"Faied GetProcAddress", APPNAME, MB_ICONEXCLAMATION);
		return IDCANCEL;
	}
	TIMEDMESSAGEBOX_PARAMS tp;
	tp.size = sizeof(tp);
	tp.flags = TIMEDMESSAGEBOX_FLAGS_POSITION | TIMEDMESSAGEBOX_FLAGS_SHOWCMD | TIMEDMESSAGEBOX_FLAGS_TOPMOST;
	tp.hWndCenterParent = NULL;
	tp.position = TIMEDMESSAGEBOX_POSITION_BOTTOMRIGHT;
	tp.nShowCmd = SW_SHOWNOACTIVATE;

	wstring title = stdFormat(L"%s %s", APPNAME, APPVERSION);
	return func2(NULL, 30, title.c_str(), pMessage, &tp);
}

class ThreadInfo {
	wstring path_;
	wstring result_;
	bool must_;
	DWORD timeout_;
public:
	ThreadInfo(const wstring& path, bool must,DWORD timeout):path_(path),timeout_(timeout) {
		result_=L"Timed out";
	}

	const wstring& path() const {
		return path_;
	}
	const wstring& result() const {
		return result_;
	}
	void setResult(const wstring& s) {
		result_=s;
	}
	bool must() const {
		return must_;
	}
	DWORD timeout()const{
		return timeout_;
	}
};
void __cdecl start(void* data)
{
	ThreadInfo* pInfo = (ThreadInfo*)data;
	try
	{
		DWORD lasttick = GetTickCount();
		int timeout = pInfo->timeout();
		do 
		{
			if (RevealFolder(pInfo->path().c_str()))
			{
				pInfo->setResult(L"OK");
				break;
			}
			else
			{
				pInfo->setResult(L"NG");
				if (!pInfo->must())
				{
					break;
				}
				else
				{
					// must
					DWORD now = GetTickCount();
					int timecost = now - lasttick;
					timeout -= timecost;
					lasttick = now;
				}
			}
		} while (timeout >= 0);
	}
	catch (...)
	{
		pInfo->setResult(L"Exception");
	}
    _endthread();
}

wstring getHelpstring()
{
	wstring out;
	out += stdGetFileName(stdGetModuleFileName()) + L"[-timeout sec] [-must] NetworkPath [[-must] NetworkPath...]";
	return out;
}
int WINAPI wWinMain(
	HINSTANCE hInstance,      // 現在のインスタンスのハンドル
	HINSTANCE hPrevInstance,  // 以前のインスタンスのハンドル
	LPWSTR lpCmdLine,          // コマンドライン
	int nCmdShow              // 表示状態
	)
{
	Ambiesoft::InitHighDPISupport();

	if (__argc <= 1)
	{
		wstring message = L"No Arguments";
		message += L"\r\n\r\n";
		message += getHelpstring();
		MessageBox(NULL, message.c_str(), APPNAME, MB_ICONEXCLAMATION);
		return 1;
	}

	wstring message;
	vector<HANDLE> handles;
	vector<ThreadInfo*> infos;

	// find timeout
	int timeout = -1;
	for (int i = 1; i < __argc; ++i)
	{
		if (_wcsicmp(__wargv[i], L"-timeout")==0)
		{
			++i;
			if (!__wargv[i])
			{
				wstring message = L"timeout must be specified";
				message += L"\r\n\r\n";
				message += getHelpstring();
				MessageBox(NULL, message.c_str(), APPNAME, MB_ICONEXCLAMATION);
				return 1;
			}
			timeout = _wtoi(__wargv[i]) * 1000;
		}
	}

	for (int i = 1; i < __argc; ++i)
	{
		if (_wcsicmp(__wargv[i], L"-timeout") == 0)
		{
			i += 2;
			continue;
		}
		bool must = false;
	
		if (_wcsicmp(__wargv[i], L"-must") == 0)
		{
			must = true;
			++i;
		}
		wchar_t buff[MAX_PATH];
		DWORD dwGFPN = GetFullPathName(__wargv[i], MAX_PATH, buff, NULL);
		if (dwGFPN > MAX_PATH)
		{
			message += __wargv[i];
			message += L" -> ";
			message += L"NG (Too long path)";
			message += L"\r\n";
			continue;
		}

		ThreadInfo* pInfo = new ThreadInfo(buff,must,timeout);
		HANDLE hThread = (HANDLE)_beginthread(start,0,pInfo);
		if(!hThread)
			pInfo->setResult(L"Failed to create a thread.");
		handles.push_back(hThread);
		infos.push_back(pInfo);
	}

	WaitForMultipleObjects(handles.size(), &handles[0], TRUE, timeout);
	for(size_t i=0; i < handles.size(); ++i)
	{
		// CloseHandle(handles[i]);
		message += infos[i]->path();
		message += L" -> ";
		message += infos[i]->result();
		message += L"\r\n";

		delete infos[i];
	}
	
	DWORD ret = ShowTimedMessage(message.c_str());
	if (ret == IDRETRY)
	{
		ReopenCommon(NULL);
	}
	return 0;
}