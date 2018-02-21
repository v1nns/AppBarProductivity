#include "stdafx.h"
#include "TaskSheet.h"

#define WM_RESIZEPAGE WM_USER + 111

CTaskSheet::CTaskSheet()
{
	AddPage(&pgeTotalTasks);
	
	SetLook(CMFCPropertySheet::PropSheetLook_OneNoteTabs);
}


CTaskSheet::~CTaskSheet()
{
}

BEGIN_MESSAGE_MAP(CTaskSheet, CMFCPropertySheet)
	ON_MESSAGE(WM_RESIZEPAGE, OnResizePage)
	ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CTaskSheet::OnInitDialog()
{
	CMFCPropertySheet::OnInitDialog();

	RECT rc;

	// resize the sheet
	GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.right += 50;
	rc.bottom += 50;
	MoveWindow(&rc);

	// resize the CTabCtrl
	CTabCtrl* pTab = GetTabControl();
	ASSERT(pTab);
	pTab->GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.right += 50;
	rc.bottom += 50;
	pTab->MoveWindow(&rc);

	// resize the page
	CMFCPropertyPage* pPage = (CMFCPropertyPage*)GetActivePage();
	ASSERT(pPage);
	// store page size in m_PageRect
	pPage->GetWindowRect(&m_PageRect);
	ScreenToClient(&m_PageRect);
	m_PageRect.right += 50;
	m_PageRect.bottom += 50;
	pPage->MoveWindow(&m_PageRect);

	// move the OK, Cancel, and Apply buttons
	CWnd* pWnd = GetDlgItem(IDOK);
	pWnd->GetWindowRect(&rc);
	rc.bottom += 50;
	rc.top += 50;
	ScreenToClient(&rc);
	pWnd->MoveWindow(&rc);

	pWnd = GetDlgItem(IDCANCEL);
	pWnd->GetWindowRect(&rc);
	rc.bottom += 50;
	rc.top += 50;
	ScreenToClient(&rc);
	pWnd->MoveWindow(&rc);

	pWnd = GetDlgItem(ID_APPLY_NOW);
	pWnd->GetWindowRect(&rc);
	rc.bottom += 50;
	rc.top += 50;
	ScreenToClient(&rc);
	pWnd->MoveWindow(&rc);

	CenterWindow();

	return TRUE;
}

LONG CTaskSheet::OnResizePage(UINT, LONG)
{
	// resize the page using m_PageRect which was set in OnInitDialog()
	CMFCPropertyPage* pPage = (CMFCPropertyPage*)GetActivePage();
	ASSERT(pPage);
	pPage->MoveWindow(&m_PageRect);

	return 0;
}

BOOL CTaskSheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pnmh = (LPNMHDR)lParam;

	// the sheet resizes the page whenever it is activated
	// so we need to resize it to what we want
	if (TCN_SELCHANGE == pnmh->code)
		// user-defined message needs to be posted because page must
		// be resized after TCN_SELCHANGE has been processed
		PostMessage(WM_RESIZEPAGE);

	return CMFCPropertySheet::OnNotify(wParam, lParam, pResult);
}

void CTaskSheet::OnApplyNow()
{
	// the sheet resizes the page whenever the Apply button is clicked
	// so we need to resize it to what we want
	PostMessage(WM_RESIZEPAGE);
}

void CTaskSheet::OnSize(UINT nType, int cx, int cy)
{
	CMFCPropertySheet::OnSize(nType, cx, cy);

	if (::IsWindow(pgeTotalTasks.m_hWnd))
	{
		CRect PSClientRect;
		GetClientRect(PSClientRect);
		GetTabControl()->MoveWindow(PSClientRect);
		SetActivePage(GetActiveIndex());
	}
}
