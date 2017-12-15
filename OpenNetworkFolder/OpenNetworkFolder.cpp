
// OpenNetworkFolder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OpenNetworkFolder.h"
#include "OpenNetworkFolderDlg.h"

#include "../../lsMisc/GetOpenFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COpenNetworkFolderApp

BEGIN_MESSAGE_MAP(COpenNetworkFolderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COpenNetworkFolderApp construction

COpenNetworkFolderApp::COpenNetworkFolderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only COpenNetworkFolderApp object

COpenNetworkFolderApp theApp;


// COpenNetworkFolderApp initialization

DWORD WINAPI sos(void* p)
{
	Ambiesoft::GetOpenFile(
		NULL,
		NULL,
		(LPCTSTR)p,
		L"",
		NULL);
	return 0;
}

// http://www.kab-studio.biz/Programing/Codian/ShellExtension/05.html
BOOL MakeList(IShellFolderPtr pSF, LPITEMIDLIST p_pFolderIDList)
{
	HRESULT		hRes;
	ULONG		ulRetNo;
	STRRET		stFileName;
	LPITEMIDLIST	pFileIDList;
	IShellFolderPtr	pCurFolder;
	IEnumIDListPtr	pEnumIDList;
	CString		cPrintStr;

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


LPITEMIDLIST GetItemIDList(IShellFolderPtr pSF, CString p_cFileStr)
{
	if (p_cFileStr.IsEmpty())
		return NULL;

	HRESULT		hRes;
	ULONG		chEaten;	//文字列のサイズを受け取ります。
	ULONG		dwAttributes;	//属性を受け取ります。
	//OLECHAR		ochPath[MAX_PATH];	//ワイドバイト文字列です。
	LPITEMIDLIST	pIDL;	//フォルダを示すアイテムＩＤです。

	//　これをしないとインターフェイスはダメなのです。
	// ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p_cFileStr, -1, ochPath, MAX_PATH);

	//　実際にITEMIDLISTを取得します。
	
	hRes = pSF->ParseDisplayName(NULL, NULL, p_cFileStr.GetBuffer(MAX_PATH), &chEaten, &pIDL, &dwAttributes);

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

	CoTaskMemFree(pSF);

	return TRUE;
}

BOOL COpenNetworkFolderApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	if (__argc <= 1)
	{
		AfxMessageBox(L"No Arguments");
		return FALSE;
	}

	CString message;
	for (int i = 1; i < __argc; ++i)
	{
		//HANDLE hFile = CreateFile(__targv[i],
		//	GENERIC_READ,
		//	FILE_SHARE_READ | FILE_SHARE_WRITE,
		//	NULL,
		//	OPEN_EXISTING,
		//	FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_DIRECTORY,
		//	NULL);
		//CloseHandle(hFile);
		message += __targv[i];
		message += L" -> ";

		if (DoReveal(__targv[i]))
		{
			message += L"OK";
		}
		else
		{
			message += L"NG";
		}
		message += L"\r\n";
		//LPTSTR p = _tcsdup(__targv[i]);
		//::CreateThread(NULL,
		//	0,
		//	sos,
		//	(void*)p,
		//	0,
		//	NULL);
	}
	// ::Sleep(30 * 1000);
	AfxMessageBox(message, MB_ICONINFORMATION);
	return FALSE;

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	COpenNetworkFolderDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

