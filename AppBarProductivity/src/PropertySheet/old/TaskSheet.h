#pragma once
#include "afxpropertysheet.h"

#include "TotalTasks.h"

class CTaskSheet : public CMFCPropertySheet
{
public:
	CTaskSheet();
	virtual ~CTaskSheet();

private:
	CTotalTasks pgeTotalTasks;

protected:
	DECLARE_MESSAGE_MAP()

protected:
	RECT m_PageRect;
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnResizePage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnApplyNow();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

