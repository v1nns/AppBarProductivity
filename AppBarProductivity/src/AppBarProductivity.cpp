
// AppBarProductivity.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AppBarProductivity.h"
#include "MainFrm.h"
#include "JWinSearch\JWinSearch\jwinsearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppBarProductivityApp

BEGIN_MESSAGE_MAP(CAppBarProductivityApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAppBarProductivityApp construction

CAppBarProductivityApp::CAppBarProductivityApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	appbar = NULL;
}


// The one and only CAppBarProductivityApp object

CAppBarProductivityApp theApp;

// CAppBarProductivityApp initialization

BOOL CAppBarProductivityApp::InitInstance()
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
	SetRegistryKey(_T("Vinicius Own-company"));

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// creates a simple frame, without caption, icon or menu
	pFrame->Create(NULL, _T("AppBarProductivity"), WS_POPUP);

	// avoid taskbar button to appear, also removes 3D edge
	pFrame->ModifyStyleEx(WS_EX_APPWINDOW | WS_EX_CLIENTEDGE, WS_EX_TOOLWINDOW);
	// Don't show, AppBar Manager class will do
	pFrame->ShowWindow(SW_HIDE);
	pFrame->UpdateWindow();

	// Create AppBar manager thread
	appbar = (CAppBarMngr *)::AfxBeginThread(RUNTIME_CLASS(CAppBarMngr));

	// Init AppBar Manager, right sided
	int result = appbar->Init(pFrame->m_hWnd, 150, true);

	// Loading thread in order to use the advanced Ctrl+tab =P
	CJWinSearch *tabSearch = (CJWinSearch*)::AfxBeginThread(RUNTIME_CLASS(CJWinSearch));
	tabSearch->Init();

	// Check if hooking has been successful
	if (result == APPBARHOOK_SUCCESS)
		return TRUE;
	else if (result == APPBARHOOK_DLLERROR)
		::AfxMessageBox(_T("Error loading AppBarHook.dll"));
	// else should be APPBARHOOK_ALREADYHOOKED, close application
	
	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CAppBarProductivityApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}

void CAppBarProductivityApp::OnPinButton(bool pin)
{
	if (appbar != NULL)
		appbar->OnPinButton(pin);
}

void CAppBarProductivityApp::OnTemporaryPin(bool pin, int timeout)
{
	if (appbar != NULL)
		appbar->OnTemporaryPin(pin, timeout);
}

bool CAppBarProductivityApp::IsButtonPinned()
{
	if (appbar != NULL)
		return appbar->IsButtonPinned();
	
	return false;
}

bool CAppBarProductivityApp::IsTemporaryPinned()
{
	if (appbar != NULL)
		return appbar->IsTemporaryPinned();

	return false;
}

BOOL CAppBarProductivityApp::PreTranslateMessage(MSG* pMsg)
{

	return CWinApp::PreTranslateMessage(pMsg);
}
