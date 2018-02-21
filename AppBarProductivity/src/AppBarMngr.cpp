// AppBarMngr.cpp : implementation file
//

#include "stdafx.h"
#include "AppBarMngr.h"

#include <time.h>
#include <thread>

/////////////////////////////////////////////////////////////////////////////
// CAppBarMngr

IMPLEMENT_DYNCREATE(CAppBarMngr, CWinThread)

CAppBarMngr::CAppBarMngr()
{
	m_Module = NULL;
	m_pWindow= NULL;
	m_Width  = 0;
	m_Left = true;

	m_IsPinnedButton = false;
	m_IsTemporaryPinned = false;
}

CAppBarMngr::~CAppBarMngr()
{
}

BOOL CAppBarMngr::InitInstance()
{
	return TRUE;
}

int CAppBarMngr::ExitInstance()
{
	if (m_Module)
		::FreeLibrary(m_Module); // closes DLL handler

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CAppBarMngr, CWinThread)
	//{{AFX_MSG_MAP(CAppBarMngr)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppBarMngr message handlers

//------------------------------------------------------------------------------------
// Function:  Init   - Loads DLL functions and initilize mouse hook
// Arguments: _hWnd  - Handler of window to manage
//            _width - Desired width of managed window
//            _left  - True if window is left side docked, false if not
// Returns:   APPBARHOOK_DLLERROR - An error has ocurred while loading DLL functions
//            APPBARHOOK_ALREADYHOOKED - Another instance has hooked mouse
//            APPBARHOOK_SUCCESS - All is OK
//------------------------------------------------------------------------------------
int CAppBarMngr::Init(HWND _hWnd, int _width, bool _left)
{
	
	ExtractResource(AfxGetInstanceHandle(), IDR_DLL_BIN, "AppBarHook.dll");

	m_Module = ::LoadLibrary("AppBarHook.dll"); // Load DLL
	if (!m_Module)
		return APPBARHOOK_DLLERROR;

	m_fpSetHook = (TFSetHook)::GetProcAddress(m_Module, "SetHook");   // Load function

	if (m_fpSetHook)                         // If function has been loaded sucessfully
	{
		m_pWindow  = CWnd::FromHandle(_hWnd);
		m_Left  = _left;                     // Set orientation and width
		m_Width = _width;

		// Get the Display area for multi-monitor usage
		HDC hdc = ::GetDC(NULL);
		::GetClipBox(hdc, &m_Screen);
		::ReleaseDC(NULL, hdc);

		// Make a first slide
		if (m_Left) 
			SlideWindow(m_Screen.left, false);
		else
			SlideWindow(m_Screen.right-m_Width, true);

		if (!(*m_fpSetHook)(this->m_nThreadID, m_Width, m_Left))  // Invoke SetHook function
		{
			::FreeLibrary(m_Module);         // Clean DLL reference
			m_Module = NULL;
			return APPBARHOOK_ALREADYHOOKED; // Already hooked, maybe second instance
		}

		return APPBARHOOK_SUCCESS;           // All is OK
	} else {
		::FreeLibrary(m_Module);             // Clean DLL reference
		m_Module = NULL;
		return APPBARHOOK_DLLERROR;
	}
}

//-----------------------------------------------------------------------------------------------
// Function:  PreTranslateMessage - Receive messages from hook module
// Arguments: pMsg  - Pointer to message
// Messages:  WM_USER+2 - Border of screen has been touched - possibly must make window to appear
//            WM_USER+3 - Mouse cursor is outside window area - possibly must make window to hide
//-----------------------------------------------------------------------------------------------
BOOL CAppBarMngr::PreTranslateMessage(MSG* pMsg) 
{
	static CRect rect;

	switch (pMsg->message) {
		case WM_USER+2:												// Activate (border touched)
			if(!m_IsPinnedButton && !m_IsTemporaryPinned)
			{
				m_pWindow->GetWindowRect(&rect);
				if (m_Left) {										// If window is left sided
					if (rect.left < m_Screen.left)					// Evaluate if window must appear
					{
						SlideWindow(m_Screen.left - m_Width, true); // Slide it from left to right
					}
				}
				else {												// If window is right sided
					if (rect.right > m_Screen.right)                // Evaluate if window must appear
						SlideWindow(m_Screen.right, false);			// Slide it from right to left
				}
			}
			break;
		case WM_USER+3:												// Deactivate (hide)
			if (!m_IsPinnedButton && !m_IsTemporaryPinned)
			{
				m_pWindow->GetWindowRect(&rect);
				if (m_Left) {										// If window is left sided
					if (rect.left >= m_Screen.left)                 // Evaluate if window must disappear
						SlideWindow(m_Screen.left, false);			// Slide it from right to left
				}
				else {												// If window is right sided
					if (rect.left < m_Screen.right)                 // Evaluate if window must disappear
						SlideWindow(m_Screen.right - m_Width, true);// Slide it from left to right
				}
			}
			break;
	}

	return CWinThread::PreTranslateMessage(pMsg);
}

//-----------------------------------------------------------------------------------------------
// Function:  SlideWindow - Slide window in any orientation smoothly
// Arguments: xStart      - Starting horizontal screen coordinate
//            left2right  - Sliding orientation, true for left to right, false for countersense
//-----------------------------------------------------------------------------------------------
void CAppBarMngr::SlideWindow(int xStart, bool left2right) 
{
	int   maxdelay = 5;                         // delay between steps (miliseconds)
	int   h = ::GetSystemMetrics(SM_CYSCREEN);  // Screen height (pixels)
	int   x;                                    // instantaneous left side position (screen coords)
	unsigned int t;                             // time for next step
	float step = (float)m_Width / 10.0f;        // step size (pixels)

	for (int i=0, delay=maxdelay; i<=10; i++, delay+=2) {
		t = ::GetTickCount() + delay;           // Calculate next next time
		x = xStart + (int)(i*step) * (left2right?1:-1);   // Step window pos
		m_pWindow->SetWindowPos(&CWnd::wndTopMost, x, 0, m_Width, h, SWP_SHOWWINDOW);  // Do move
		while (::GetTickCount()<t)              // Wait for next step
			::Sleep(1);
	}
}

//------------------------------------------------------------------------------------
// Function:  ExtractResource - Find a binary file and extract it to the format you desire
// Arguments: hInstance  - Current Instance
//            resourceID - File's resource ID
//            szFilename - Target filepath including name ("filename.extension")
// Returns:   false - An error has ocurred while extracting resource
//            true  - All is OK
//------------------------------------------------------------------------------------
bool CAppBarMngr::ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCTSTR szFilename)
{
	bool bSuccess = false;
	try
	{
		// Find and load the resource
		HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), _T("BINARY"));
		HGLOBAL hFileResource = LoadResource(hInstance, hResource);

		// Open and map this to a disk file
		LPVOID lpFile = LockResource(hFileResource);
		DWORD dwSize = SizeofResource(hInstance, hResource);

		// Open the file and filemap
		HANDLE hFile = CreateFile(szFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, dwSize, NULL);
		LPVOID lpAddress = MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);

		// Write the file
		CopyMemory(lpAddress, lpFile, dwSize);

		// Un-map the file and close the handles
		UnmapViewOfFile(lpAddress);
		CloseHandle(hFileMap);
		CloseHandle(hFile);
	}
	catch (...)
	{
		// Whatever
	}
	return bSuccess;
}

//
void CAppBarMngr::OnPinButton(bool pin)
{
	m_IsPinnedButton = pin;
}

static void CALLBACK TimerProc(void* param, BOOLEAN timerCalled)
{
	CAppBarMngr *thread = (CAppBarMngr*)param;
	thread->OnTemporaryPin(false);
};

void CAppBarMngr::OnTemporaryPin(bool pin, int timeout)
{
	m_IsTemporaryPinned = pin;

	if (!m_IsPinnedButton && m_IsTemporaryPinned && timeout > 0)
	{
		HANDLE hTimer;
		BOOL success = CreateTimerQueueTimer(&hTimer,
			NULL,
			TimerProc,
			this,
			timeout,
			0,
			WT_EXECUTEINTIMERTHREAD);		
	}
}

bool CAppBarMngr::IsButtonPinned()
{
	return m_IsPinnedButton;
}

bool CAppBarMngr::IsTemporaryPinned()
{
	return m_IsTemporaryPinned;
}