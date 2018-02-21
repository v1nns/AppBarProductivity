#pragma once

#include "Resource.h"
#include "afxwin.h"
#include "afxdtctl.h"

// CInsertTaskDialog dialog
class CInsertTaskDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CInsertTaskDialog)

public:
	CInsertTaskDialog(CWnd* pParent = NULL);   // standard constructor
	CInsertTaskDialog(COleDateTime& date, CWnd* pParent = NULL);   // date constructor

	virtual ~CInsertTaskDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INSERT_TASK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonTaskAdd();

public:
	CString GetTaskName();
	COleDateTime GetTime();

// Attributes
private:
	CEdit m_TaskName;
	CDateTimeCtrl m_TaskDate;
	CButton m_BtnAdd;

	COleDateTime m_TempDate;

	// Info
	CString m_StrTask;
	COleDateTime m_Date;
	virtual void OnOK();
};
