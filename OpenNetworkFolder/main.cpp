
#include <Windows.h>



#include <string>

#include "../../TimedMessageBox/TimedMessageBox/TimedMessageBox.h"
#include "../../lsMisc/RevealFolder.h"


using namespace std;
using namespace Ambiesoft;



#define APPNAME L"OpenNetworkFolder"
void ShowTimedMessage(LPCTSTR pMessage)
{
	HMODULE hModule = LoadLibrary(L"TimedMessageBox.dll");
	if (!hModule)
	{
		MessageBox(NULL, L"Failed to load TimedMessageBox.dll", APPNAME, MB_ICONEXCLAMATION);
		return;
	}
	FNTimedMessageBox2 func2 = NULL;
	func2 = (FNTimedMessageBox2)GetProcAddress(hModule, "fnTimedMessageBox2");
	if (!func2)
	{
		MessageBox(NULL, L"Faied GetProcAddress", APPNAME, MB_ICONEXCLAMATION);
		return;
	}
	TIMEDMESSAGEBOX_PARAMS tp;
	tp.size = sizeof(tp);
	tp.flags = TIMEDMESSAGEBOX_FLAGS_POSITION | TIMEDMESSAGEBOX_FLAGS_SHOWCMD | TIMEDMESSAGEBOX_FLAGS_TOPMOST;
	tp.hWndCenterParent = NULL;
	tp.position = TIMEDMESSAGEBOX_POSITION_BOTTOMRIGHT;
	tp.nShowCmd = SW_SHOWNOACTIVATE;
	func2(NULL, 10, APPNAME, pMessage, &tp);
}


int WINAPI wWinMain(
	HINSTANCE hInstance,      // 現在のインスタンスのハンドル
	HINSTANCE hPrevInstance,  // 以前のインスタンスのハンドル
	LPWSTR lpCmdLine,          // コマンドライン
	int nCmdShow              // 表示状態
	)
{
	if (__argc <= 1)
	{
		MessageBox(NULL, L"No Arguments", APPNAME, MB_ICONEXCLAMATION);
		return FALSE;
	}

	wstring message;
	for (int i = 1; i < __argc; ++i, message += L"\r\n")
	{
		message += __wargv[i];
		message += L" -> ";

		wchar_t buff[MAX_PATH];
		DWORD dwGFPN = GetFullPathName(__wargv[i], MAX_PATH, buff, NULL);
		if (dwGFPN > MAX_PATH)
		{
			message += L"NG (Too long path)";
			continue;
		}

		if (RevealFolder(buff))
		{
			message += L"OK";
		}
		else
		{
			message += L"NG";
		}
	}
	ShowTimedMessage(message.c_str());
	return 0;
}