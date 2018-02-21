#if !defined(AFX_APPBARMNGR_H__9222FF0A_147A_4534_BB51_FD99FE14D4BD__INCLUDED_)
#define AFX_APPBARMNGR_H__9222FF0A_147A_4534_BB51_FD99FE14D4BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// AppBarMngr.h : header file
//
#include "resource.h"

// Imported function declarations
typedef BOOL (*TFSetHook)(DWORD id, int width, BOOL left);
typedef BOOL (*TFUnSetHook)();

// Init() return values
#define APPBARHOOK_SUCCESS				0
#define APPBARHOOK_ALREADYHOOKED		1
#define APPBARHOOK_DLLERROR				2

#define APPBAR_TIMEOUT_TEMPORARY_PIN	100

// AppBar Manager class declaration
class CAppBarMngr : public CWinThread
{
	DECLARE_DYNCREATE(CAppBarMngr)
public:
	CAppBarMngr();           // protected constructor used by dynamic creation

// Attributes
public:
	HMODULE     m_Module;			// Handler of Hook DLL
	TFSetHook   m_fpSetHook;		// Pointer to function in DLL
	CWnd *      m_pWindow;			// Pointer to managed window
	int         m_Width;			// Desired width for window
	bool        m_Left;				// True if window is left side docked, false if not
	CRect       m_Screen;			// coordinates of full screen including multi-monitors
	bool		m_IsPinnedButton;	// Pin button is pressed in order to maintain App Bar visible
	bool		m_IsTemporaryPinned;// Maintain App Bar temporary visible 

// Operations
public:
	int Init(HWND _hWnd, int _width, bool _left);
	void SlideWindow(int xStart, bool left2right); 
	bool ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCTSTR szFilename);
	
	void OnPinButton(bool pin = true);
	void OnTemporaryPin(bool pin = true, int timeout = 0);

	bool IsButtonPinned();
	bool IsTemporaryPinned();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppBarMngr)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAppBarMngr();

	// Generated message map functions
	//{{AFX_MSG(CAppBarMngr)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPBARMNGR_H__9222FF0A_147A_4534_BB51_FD99FE14D4BD__INCLUDED_)
