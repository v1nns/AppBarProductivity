// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__0BAAEE30_BBE0_4BED_AC03_EA45087CEC77__INCLUDED_)
#define AFX_MAINFRM_H__0BAAEE30_BBE0_4BED_AC03_EA45087CEC77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#include "TaskLoader.h"
#include "CatListBox.h"
#include "InsertTaskDialog.h"

#include "PushPin.h"
#include "BtnST.h"

#include "FPSMiniCalendar\FPSMiniCalendarCtrl.h"
#include "WMDateCtrl\WMDateCtrl.h"

#include "ScintillaEditor\ScintillaWnd.h"

#include "PropertySheet\EnTabCtrl.h"
#include "PropertySheet\TabCtrl\TabCtrl.h"

#include "ToolTip\XSuperTooltip.h"

#define ID_RESOURCE_PINBUTTON			200
#define ID_RESOURCE_SEARCHEDIT			201
#define ID_RESOURCE_CALENDAR			202
#define ID_RESOURCE_CALENDARBUTTON		203

#define ID_RESOURCE_NOTEPAD				300
#define ID_RESOURCE_NOTEPAD_NEWPAGE		301

#define ID_RESOURCE_AGENDA				400
#define ID_RESOURCE_AGENDA_PAGE_1		401
#define ID_RESOURCE_AGENDA_PAGE_2		402

#define	APPBAR_INSERTTASK				WM_APP + 10000
#define	APPBAR_LOADTASKTREE				WM_APP + 10001
#define	APPBAR_SAVETASKTREE				WM_APP + 10002

#define SIZE_VALUE_SEARCHEDIT			30
#define SIZE_VALUE_CALENDAR				130
#define SIZE_VALUE_TABCONTROLTASKS		365
#define SIZE_VALUE_TABCONTROLNOTEPAD	130

class CMainFrame : public CFrameWnd, public TabCtrlNotify
{	
public:
	CMainFrame();
	virtual ~CMainFrame();

protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Operations
protected:
	void EraseBackground(CDC *pDC);
		
	void DrawNotepadBkgnd(CDC *pDC);
	void DrawAgendaBkgnd(CDC *pDC);

// Agenda
private:
	void InitializeAgenda();

	void OnInsertTask();
	void OnLoadAgenda();
	void OnSaveAgenda();
	
	void OnUpdateAgendaTabs();

// Notepad
private:
	void InitializeNotepad();

// Pin Button
private:
	void InitializePinButton();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG

	afx_msg void OnDynamicButtonClicked();
	
	afx_msg void OnCalendarDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnCalendarHoveringToday(WPARAM wparam, LPARAM lparam);

	afx_msg void OnTabSaveDoc();

	DECLARE_MESSAGE_MAP()

private: // TabCtrlNotify.
	virtual void OnCloseButtonClicked(TabCtrl *pCtrl, CRect const *pRect, CPoint ptScr);
	virtual void OnMenuButtonClicked(TabCtrl *pCtrl, CRect const *pRect, CPoint ptScr);
	virtual void OnAddTabButtonClicked(TabCtrl * pCtrl, CRect const * pRect, CPoint ptScr);
	virtual void OnDrag(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr, bool outside);

// Attributes
private:
	//Mutex
	CMutex* m_pToolTipMutex;

	//Calendar
	CFPSMiniCalendarCtrl m_Calendar;
	CRect m_RectToday;
	bool m_IsShowingTip;

	//Tasks
	std::vector<Task> m_ListTasks;
	CCatListBox m_TabTotal, m_TabMonthly;
	TabCtrl m_TabControlTasks;
	TabCtrlStyle_VS2010_bars style;
	//TabCtrlStyle_VS2008_bars_silver style;
	std::vector<Task> m_TodayTaskList;

	//Notepad
	HINSTANCE   m_hDll;
	std::vector<CScintillaWnd*> m_ListNotepad;
	TabCtrl m_TabControlCustomPad;
	HACCEL m_hAccelNotepad;      // handle to accelerator table 
	
	// Pin Button
	CPushPinButton m_PinButton;

	// Calendar Button
	CButtonST m_CalendarButton;

	// Tooltip
	CXSuperTooltip m_Tip;

	// Variables
	bool m_isPinned;

	int m_Palette;
	CBrush m_ClearColor, m_WhiteColor;

	bool m_bPoint;
	
	CFont m_FontTitle, m_FontSubtitle, m_FontText, m_FontTasks;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__0BAAEE30_BBE0_4BED_AC03_EA45087CEC77__INCLUDED_)
