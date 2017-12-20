
#include <Windows.h>

#include <Shlobj.h>
#include <Shobjidl.h>
#include <comdef.h>
#include <comdefsp.h>

#include <string>

#include "../../TimedMessageBox/TimedMessageBox/TimedMessageBox.h"

using namespace std;

// http://www.kab-studio.biz/Programing/Codian/ShellExtension/05.html
BOOL MakeList(IShellFolderPtr pSF, LPITEMIDLIST p_pFolderIDList)
{
	HRESULT		hRes;
	ULONG		ulRetNo;
	STRRET		stFileName;
	LPITEMIDLIST	pFileIDList;
	IShellFolderPtr	pCurFolder;
	IEnumIDListPtr	pEnumIDList;
	wstring		cPrintStr;

	if (p_pFolderIDList != NULL)
	{
		//　IShellFolderにバインドします。
		hRes = pSF->BindToObject(p_pFolderIDList, NULL, IID_IShellFolder, (LPVOID *)&pCurFolder);
		if (hRes != NOERROR)
			return TRUE;
	}
	else
	{
		//　デスクトップフォルダを指定します。
		hRes = ::SHGetDesktopFolder(&pCurFolder);
		if (hRes != NOERROR)
			return TRUE;
	}

	//　IEnumIDListを取得します。
	hRes = pCurFolder->EnumObjects(NULL //GetSafeHwnd()
		, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pEnumIDList);
	if (hRes != NOERROR)
		return FALSE;

	//　IEnumIDListからアイテムＩＤを取得していきます。
	while (pEnumIDList->Next(1, &pFileIDList, &ulRetNo) == NOERROR)
	{
		//　ファイルパスの取得。
		hRes = pCurFolder->GetDisplayNameOf(pFileIDList
			, SHGDN_FORPARSING, &stFileName);
		if (hRes != NOERROR)
			break;

		//　文字列の変換。
		// cPrintStr = TFileName(pFileIDList, &stFileName);

		// TRACE("%s\n", (LPCTSTR)cPrintStr);

		CoTaskMemFree(pFileIDList);
	}

	//pCurFolder->Release();

	return TRUE;
}


LPITEMIDLIST GetItemIDList(IShellFolderPtr pSF, wstring p_cFileStr)
{
	if (p_cFileStr.empty())
		return NULL;

	HRESULT		hRes;
	ULONG		chEaten;	//文字列のサイズを受け取ります。
	ULONG		dwAttributes;	//属性を受け取ります。
	//OLECHAR		ochPath[MAX_PATH];	//ワイドバイト文字列です。
	LPITEMIDLIST	pIDL;	//フォルダを示すアイテムＩＤです。

	//　これをしないとインターフェイスはダメなのです。
	// ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p_cFileStr, -1, ochPath, MAX_PATH);

	//　実際にITEMIDLISTを取得します。

	hRes = pSF->ParseDisplayName(NULL, NULL, (LPWSTR)p_cFileStr.c_str(), &chEaten, &pIDL, &dwAttributes);

	if (hRes != NOERROR)
		pIDL = NULL;

	return pIDL;	//取得したアイテムＩＤを返します。
}

BOOL DoReveal(LPCTSTR pFolder)
{
	IShellFolderPtr pSF;
	HRESULT hRes = ::SHGetDesktopFolder(&pSF);
	if (!pSF)
		return FALSE;

	LPITEMIDLIST pIIL = GetItemIDList(pSF, pFolder);
	if (!pIIL)
		return FALSE;

	if (!MakeList(pSF, pIIL))
		return FALSE;

	return TRUE;
}


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

		if (DoReveal(buff))
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