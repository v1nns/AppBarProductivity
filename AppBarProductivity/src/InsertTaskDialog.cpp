// InsertTaskDialog.cpp : implementation file
//

#include "stdafx.h"
#include "InsertTaskDialog.h"

// CInsertTaskDialog dialog

IMPLEMENT_DYNAMIC(CInsertTaskDialog, CDialogEx)

CInsertTaskDialog::CInsertTaskDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INSERT_TASK, pParent)
{
	m_TempDate = COleDateTime::GetCurrentTime();
}

CInsertTaskDialog::CInsertTaskDialog(COleDateTime& date, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INSERT_TASK, pParent)
{
	m_TempDate = date;
}

CInsertTaskDialog::~CInsertTaskDialog()
{
}

void CInsertTaskDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TASK, m_TaskName);
	DDX_Control(pDX, IDC_DATETIMEPICKER_TASK, m_TaskDate);
	DDX_Control(pDX, IDC_BUTTON_TASK_ADD, m_BtnAdd);
}

BEGIN_MESSAGE_MAP(CInsertTaskDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_TASK_ADD, &CInsertTaskDialog::OnBnClickedButtonTaskAdd)
END_MESSAGE_MAP()


// CInsertTaskDialog message handlers


BOOL CInsertTaskDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_TaskDate.SetTime(m_TempDate);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CInsertTaskDialog::OnBnClickedButtonTaskAdd()
{
	OnOK();
}

CString CInsertTaskDialog::GetTaskName()
{
	return m_StrTask;
}

COleDateTime CInsertTaskDialog::GetTime()
{
	return m_Date;
}

void CInsertTaskDialog::OnOK()
{
	CString tmpStrTask;
	m_TaskName.GetWindowTextA(tmpStrTask);

	if (tmpStrTask.GetLength() > 2)
	{
		m_StrTask = tmpStrTask;
		m_TaskDate.GetTime(m_Date);
		CDialogEx::OnOK();
	}
	else
	{
		//TOOLTIP telling to write a little bit more
		CDialogEx::OnCancel();
	}
}
