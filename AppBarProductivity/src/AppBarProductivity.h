// AppBarProductivity.h : main header file for the PROJECT_NAME application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "AppBarMngr.h"

// CAppBarProductivityApp:
// See AppBarProductivity.cpp for the implementation of this class
//
class CAppBarProductivityApp : public CWinApp
{
public:
	CAppBarProductivityApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void OnPinButton(bool pin = true);
	void OnTemporaryPin(bool pin = true, int timeout = 0);

	bool IsButtonPinned();
	bool IsTemporaryPinned();

// Implementation
	DECLARE_MESSAGE_MAP()

private:
	CAppBarMngr *appbar;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

extern CAppBarProductivityApp theApp;