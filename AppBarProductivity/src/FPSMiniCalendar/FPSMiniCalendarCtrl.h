// CFPSMiniCalendarCtrl
// Author:	Matt Gullett
//			gullettm@yahoo.com
// Copyright (c) 2001
//
// This is a user-interface componenet similar to the MS outlook mini
// calendar control.  (The one used in date picker control and the
// appointment (day view)).
//
// You may freely use this source code in personal, freeware, shareware
// or commercial applications provided that 1) my name is recognized in the
// code and if this code represents a substantial portion of the application
// that my name be included in the credits for the application (about box, etc)
//
// Use this code at your own risk.  This code is provided AS-IS.  No warranty
// is granted as to the correctness, usefulness or value of this code.
//
// Special thanks to Keith Rule for the CMemCustomDC class
// http://www.codeproject.com/gdi/flickerfree.asp
//
// *******************************************************************
// TECHNICAL NOTES:
//
// See .cpp file for tech notes
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_FPSMINICALENDARCTRL_H__53B94A35_03B0_4521_8F07_06812F1E8208__INCLUDED_)
#define AFX_FPSMINICALENDARCTRL_H__53B94A35_03B0_4521_8F07_06812F1E8208__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FPSMiniCalendarCtrl.h : header file
//

#include "MemDC.h"

// custom window styles
#define FMC_MULTISELECT			0x0040L
#define FMC_NOHIGHLIGHTTODAY	0x0080L		
#define FMC_TODAYBUTTON			0x0100L	
#define FMC_NONEBUTTON			0x0200L	
#define FMC_AUTOSETTINGS		0x0400L
#define FMC_NO3DBORDER			0x0800L
#define FMC_NOSHOWNONMONTHDAYS	0x1000L


// I usually don't use preprocessor macros like this
// but it makes sense here
#define FMC_MAX_SIZE_NONE		CSize(-2,-2)
#define FMC_MAX_SIZE_PARENT		CSize(-1,-1)

// default font settings
#define FMC_DEFAULT_FONT			"Tahoma"
#define FMC_DEFAULT_FONT_SIZE		8
#define FMC_DEFAULT_FONT_SIZE_640	7
#define FMC_DEFAULT_FONT_SIZE_800	8
#define FMC_DEFAULT_FONT_SIZE_1024	10
#define FMC_DEFAULT_FONT_SIZE_1280	12
#define FMC_DEFAULT_MIN_FONT_SIZE	5

// used to disable Maximum selection day rules
#define FMC_NO_MAX_SEL_DAYS			-1

// callback function (definition) for the IsSpecialDate
// function
typedef BOOL (CALLBACK* funcSPECIALDATE)(COleDateTime&);

#define FMC_APP_IS_MOUSE_HOVERING_TODAY	WM_APP + 1

// forward declration for list popup
class CFPSMiniCalendarListCtrl;
/////////////////////////////////////////////////////////////////////////////
// CFPSMiniCalendarCtrl window

class CFPSMiniCalendarCtrlFontInfo
{
public:
	virtual void CreateFont(CDC* pDC);
	CFPSMiniCalendarCtrlFontInfo() {m_pFont = NULL;}
	~CFPSMiniCalendarCtrlFontInfo() {if (m_pFont) delete m_pFont;}

	CString m_strFontName;
	int m_iFontSize;
	BOOL m_bBold;
	BOOL m_bItalic;
	BOOL m_bUnderline;
	COLORREF m_cColor;
	CFont*	m_pFont;
};

class CFPSMiniCalendarCtrlFontHotSpot
{
public:
	CFPSMiniCalendarCtrlFontHotSpot() {};
	~CFPSMiniCalendarCtrlFontHotSpot() {};

	CRect			m_rect;
	COleDateTime	m_dt;
};

class CFPSMiniCalendarCtrlCell
{
public:
	CFPSMiniCalendarCtrlCell();
	~CFPSMiniCalendarCtrlCell();

	CRect m_rectHeader;
	int	  m_iRow;
	int	  m_iCol;

	CRect m_rect;
	
	CFPSMiniCalendarCtrlFontHotSpot* m_parHotSpots;
};

class CFPSMiniCalendarCtrl : public CWnd
{
// Construction
public:
	CFPSMiniCalendarCtrl();

// Attributes
public:

// Operations
public:
	BOOL IsDateSelected(COleDateTime& dt);
	void AutoConfigure();
	void AutoSize();
	int ComputeTodayNoneHeight();
	CSize ComputeTotalSize();
	CSize ComputeSize();
	CSize Compute3DBorderSize();
	void FireNoneButton();
	void FireNotifyDblClick();
	void FireNotifyClick();
	void FireTodayButton();
	void ScrollRight(int iCount = 1);
	void ScrollLeft(int iCount = 1);
	void ClearSelections();
	CFPSMiniCalendarCtrlFontHotSpot* HitTest(POINT& pt);
	CFPSMiniCalendarCtrlCell* HeaderHitTest(POINT& point);

	COleDateTime GetDate();
	void SetDate(COleDateTime dt);

	void GetDateSel(COleDateTime& dtBegin, COleDateTime& dtEnd);
	void SetDateSel(COleDateTime dtBegin, COleDateTime dtEnd);

	void SetRowsAndColumns(int iRows, int iCols);
	int GetRows() {return m_iRows;}
	int GetCols() {return m_iCols;}

	void SetShowNonMonthDays(BOOL bValue) {m_bShowNonMonthDays = bValue;}
	BOOL GetShowNonMonthDays() {return m_bShowNonMonthDays;}

	void SetDefaultMinFontSize(int iValue);
	int GetDefaultMinFontSize() {return m_iDefaultMinFontSize;}

	void SetDefaultFont(LPCTSTR lpszValue);
	CString GetDefaultFont() {return m_strDefaultFontName;}

	void SetMonthName(int iMonth, LPCTSTR lpszName);
	CString GetMonthName(int iMonth);

	void SetDayOfWeekName(int iDayOfWeek, LPCTSTR lpszName);
	CString GetDayOfWeekName(int iDayOfWeek);

	void SetFirstDayOfWeek(int iDayOfWeek);
	int GetFirstDayOfWeek();

	void SetCurrentMonthAndYear(int iMonth, int iYear);
	int GetCurrentMonth() {return m_iCurrentMonth;}
	int GetCurrentYear() {return m_iCurrentYear;}

	void SetMultiSelect(BOOL bValue) {m_bMultiSel = bValue;}
	BOOL IsMultiSelect() {return m_bMultiSel;}

	void SetBackColor(COLORREF cColor) {m_cBackColor = cColor;}
	COLORREF GetBackColor() {return m_cBackColor;}

	void SetHighlightToday(BOOL bValue) {m_bHighlightToday = bValue;}	
	BOOL GetHighlightToday() {return m_bHighlightToday;}
	
	void SetShowTodayButton(BOOL bValue) {m_bShowTodayButton = bValue;}
	BOOL GetShowTodayButton() {return m_bShowTodayButton;}

	void SetShowNoneButton(BOOL bValue) {m_bShowNoneButton = bValue;}
	BOOL GetShowNoneButton() {return m_bShowNoneButton;}

	void SetNonMonthDayColor(COLORREF cColor) {m_cNonMonthDayColor = cColor;}
	COLORREF GetNonMonthDayColor(COLORREF cColor) {m_cNonMonthDayColor = cColor;}

	void SetShow3DBorder(BOOL bValue) {m_bShow3dBorder = bValue;}
	BOOL GetShow3DBorder() {return m_bShow3dBorder;}

	void SetMaxSize(SIZE size);
	CSize GetMaxSize() {return m_szMaxSize;}

	void SetUseAutoSettings(BOOL bValue) {m_bUseAutoSettings = bValue;}
	BOOL GetUseAutoSettings() {return m_bUseAutoSettings;}

	void SetMaxSelDays(int iValue) {m_iMaxSelDays = iValue;}
	int GetMaxSelDays() {return m_iMaxSelDays;}

	void SetSpecialDaysCallback(funcSPECIALDATE pValue);
	funcSPECIALDATE GetSpecialDaysCallback() {return m_pfuncCallback;}

	void SetHeaderFont(LPCTSTR lpszFont, int iSize, BOOL bBold, BOOL bItalic, BOOL bUnderline, COLORREF cColor);
	CFPSMiniCalendarCtrlFontInfo& GetHeaderFont() {return m_HeaderFontInfo;}

	void SetDaysOfWeekFont(LPCTSTR lpszFont, int iSize, BOOL bBold, BOOL bItalic, BOOL bUnderline, COLORREF cColor);
	CFPSMiniCalendarCtrlFontInfo& GetDaysOfWeekFont() {return m_DaysOfWeekFontInfo;}

	void SetSpecialDaysFont(LPCTSTR lpszFont, int iSize, BOOL bBold, BOOL bItalic, BOOL bUnderline, COLORREF cColor);
	CFPSMiniCalendarCtrlFontInfo& GetSpecialDaysFont() {return m_SpecialDaysFontInfo;}

	void SetDaysFont(LPCTSTR lpszFont, int iSize, BOOL bBold, BOOL bItalic, BOOL bUnderline, COLORREF cColor);
	CFPSMiniCalendarCtrlFontInfo& GetDaysFont() {return m_DaysFontInfo;}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFPSMiniCalendarCtrl)
	public:
	BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~CFPSMiniCalendarCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFPSMiniCalendarCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDevModeChange(LPTSTR lpDeviceName);
	afx_msg void OnFontChange();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnTimeChange();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int								m_iCurrentMonth;
	int								m_iCurrentYear;
	CFPSMiniCalendarCtrlFontInfo	m_HeaderFontInfo;
	CFPSMiniCalendarCtrlFontInfo	m_DaysOfWeekFontInfo;
	CFPSMiniCalendarCtrlFontInfo	m_DaysFontInfo;
	CFPSMiniCalendarCtrlFontInfo	m_SpecialDaysFontInfo;
	COLORREF						m_cBackColor;
	BOOL							m_bHighlightToday;
	BOOL							m_bShowTodayButton;
	BOOL							m_bShowNoneButton;
	COLORREF						m_cNonMonthDayColor;
	CString							m_arMonthNames[12];
	CString							m_arDaysOfWeekNames[7];
	int								m_iFirstDayOfWeek;
	int								m_iRows;
	int								m_iCols;
	BOOL							m_bMultiSel;
	BOOL							m_bShow3dBorder;
	BOOL							m_bUseAutoSettings;
	CSize							m_szMaxSize;
	CString							m_strDefaultFontName;
	int								m_iDefaultMinFontSize;
	int								m_iMaxSelDays;
	BOOL							m_bShowNonMonthDays;
	funcSPECIALDATE					m_pfuncCallback;

	BOOL							m_bTracking;
	COleDateTime					m_dtTrackingStart;
	BOOL							m_bScrollTracking;
	BOOL							m_bTodayTracking;
	BOOL							m_bTodayDown;
	BOOL							m_bNoneTracking;
	BOOL							m_bNoneDown;
	BOOL							m_bHeaderTracking;
	CFPSMiniCalendarListCtrl*		m_pHeaderList;
	UINT							m_iHeaderTimerID;
	CFPSMiniCalendarCtrlCell*		m_pHeaderCell;

	CRect							m_rectScrollLeft;
	CRect							m_rectScrollRight;
	CRect							m_rectToday;
	CRect							m_rectNone;

protected:
	BOOL IsSpecialDate(COleDateTime& dt);
	void AllocateCells();
	void SetCellHeaderPosition(int iMonthRow, int iMonthCol, RECT rect);
	void SetCellPosition(int iMonthRow, int iMonthCol, RECT rect);
	void SetHotSpot(int iMonthRow, int iMonthCol, int iDayCounter, COleDateTime& dt, RECT rect);
	void ClearHotSpots();
	int DrawTodayButton(CDC& dc, int iY);
	BOOL IsToday(COleDateTime& dt);
	CString CStr(long lValue);
	void CreateFontObjects();
	int DrawHeader(CDC& dc, int iY, int iLeftX, int iRow, int iCol, int iMonth, int iYear);
	int DrawDays(CDC& dc, int iY, int iLeftX, int iRow, int iCol, int iMonth, int iYear);
	int DrawDaysOfWeek(CDC& dc, int iY, int iLeftX, int iRow, int iCol);

	// computed values of importance
	BOOL							m_bFontsCreated;
	BOOL							m_bSizeComputed;
	int								m_iHeaderHeight;
	int								m_iIndividualDayWidth;
	int								m_iDaysOfWeekHeight;
	int								m_iDaysHeight;
	CSize							m_szMonthSize;

	// cells
	COleDateTime					m_dtSelBegin;
	COleDateTime					m_dtSelEnd;
	CFPSMiniCalendarCtrlCell*		m_parCells;
	int								m_iCells;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FPSMINICALENDARCTRL_H__53B94A35_03B0_4521_8F07_06812F1E8208__INCLUDED_)
