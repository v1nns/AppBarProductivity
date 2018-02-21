// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "math.h"
#include "AppBarProductivity.h"
#include "ContextMenu.h"

#define DISPLAY_WIDTH 11
#pragma warning(disable : 4244)

COLORREF palette[4][5] = {
	{ RGB(255,0,0),     RGB(255,127,127), RGB(255,191,191), RGB(255,207,207), RGB(255,239,239) },
	{ RGB(0,207,0),     RGB(63,207,63),   RGB(191,255,191), RGB(199,255,199), RGB(239,255,239) },
	{ RGB(0,0,255),     RGB(127,127,255), RGB(191,191,255), RGB(207,207,255), RGB(239,239,255) },
	{ RGB(127,127,127), RGB(175,175,175), RGB(207,207,207), RGB(217,217,217), RGB(239,239,239) },
};

enum tones { tone0=0, tone127, tone191, tone207, tone239 };

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEWHEEL()
	/* custom */
	ON_BN_CLICKED(ID_RESOURCE_PINBUTTON, OnDynamicButtonClicked)

	ON_NOTIFY(NM_DBLCLK, ID_RESOURCE_CALENDAR, OnCalendarDblClk)
	
	/* gambiarra */
	ON_COMMAND(APPBAR_INSERTTASK,	OnInsertTask)
	ON_COMMAND(APPBAR_LOADTASKTREE, OnLoadAgenda)
	ON_COMMAND(APPBAR_SAVETASKTREE, OnSaveAgenda)

	ON_COMMAND(IDR_TABCTRL_SAVEDOC, OnTabSaveDoc)
	
	ON_MESSAGE(FMC_APP_IS_MOUSE_HOVERING_TODAY, OnCalendarHoveringToday)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{	
	m_Palette = 2;
	m_ClearColor.CreateSolidBrush(palette[m_Palette][tone239]);

	m_WhiteColor.CreateSolidBrush(RGB(255, 255, 255));

	m_FontTitle.CreateFont(20, 0, 0, 0, FW_HEAVY, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, "Verdana");//"Lucida Sans Typewriter");

	m_FontSubtitle.CreateFont(14, 0, 0, 0, FW_HEAVY, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, "Lucida Sans Typewriter");

	m_FontText.CreateFont(14, 0, 0, 0, FW_NORMAL, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, /*"MS Shell Dlg");*/ "Lucida Sans Typewriter");

	m_FontTasks.CreateFont(15, 0, 0, 0, FW_NORMAL, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, "MS Shell Dlg");

	m_isPinned = false;
	m_IsShowingTip = false;

	m_pToolTipMutex = new CMutex;

	m_hDll = CScintillaWnd::LoadScintillaDll();
}

CMainFrame::~CMainFrame()
{
	CTaskLoader::GetInstance().ReleaseResources();

	// unload scintilla dll
	if (m_hDll != NULL)
		AfxFreeLibrary(m_hDll);

	if (m_ListNotepad.size() > 0)
	{
		std::vector<CScintillaWnd*>::iterator it;

		for (it = m_ListNotepad.begin(); it != m_ListNotepad.end(); it++)
		{
			((CScintillaWnd*)*it)->DestroyWindow();
			delete *it;
		}
	}

	if (m_pToolTipMutex != NULL)
		delete m_pToolTipMutex;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccelNotepad)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccelNotepad, pMsg))
			return(TRUE);
	}

	if (pMsg->message == WM_MOUSEMOVE)
	{
		CSingleLock lock(m_pToolTipMutex);
		lock.Lock();

		if (m_IsShowingTip)
		{
			if (!m_RectToday.PtInRect(pMsg->pt))
			{
				m_Tip.HideTooltip();
				m_IsShowingTip = false;
			}
		}

		lock.Unlock();
	}

	return __super::PreTranslateMessage(pMsg);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	/* Calendar */
	m_Calendar.Create(NULL, NULL, WS_CHILD | WS_VISIBLE /*| FMC_MULTISELECT | FMC_AUTOSETTINGS */| FMC_NO3DBORDER, CRect(0, 0, 0, 0), this, ID_RESOURCE_CALENDAR);
	m_Calendar.SetBackColor(RGB(255, 255, 255));

	/* Notepad */
	InitializeNotepad();
		
	/* Agenda */
	InitializeAgenda();

	/* Pin button */
	InitializePinButton();

	/* Tooltip */
	m_Tip.Create(this);

	m_CalendarButton.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_RESOURCE_CALENDARBUTTON);
	m_CalendarButton.SetIcon(IDI_ICON_CALENDAR);

	return FALSE;
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CDC NoFlick;
	CBitmap bmp, *oldBmp;
	CRect rect;

	NoFlick.CreateCompatibleDC(&dc);
	GetClientRect(&rect);
	bmp.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	oldBmp = NoFlick.SelectObject(&bmp);

	NoFlick.SelectStockObject(NULL_PEN);
	NoFlick.SelectStockObject(SYSTEM_FONT);
	NoFlick.SetBkMode(TRANSPARENT);

	EraseBackground(&NoFlick);

	NoFlick.SetTextColor(palette[m_Palette][tone0]);
	
	DrawAgendaBkgnd(&NoFlick);
	DrawNotepadBkgnd(&NoFlick);
	
	dc.BitBlt(0, 0, rect.right, rect.bottom, &NoFlick, 0, 0, SRCCOPY);
	NoFlick.SelectObject(*oldBmp);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

HBRUSH CMainFrame::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFrameWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetBkColor(palette[m_Palette][tone239]);
	int ctrlID = pWnd->GetDlgCtrlID();
	
	//if (ctrlID == m_SearchEdit.GetDlgCtrlID())
	//{
	//	//pDC->SetBkMode(TRANSPARENT);
	//	pDC->SetBkColor(RGB(255,255,255));
	//	return (HBRUSH)GetStockObject(NULL_BRUSH);
	//}

	return hbr;
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	lpMMI->ptMinTrackSize = CPoint(150,480);
	lpMMI->ptMaxTrackSize = lpMMI->ptMaxSize = CPoint(150,1600);
	
	CFrameWnd::OnGetMinMaxInfo(lpMMI);
}

void CMainFrame::InitializeAgenda()
{
	//Create TabControl and its childs
	m_TabControlTasks.Create(this, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), ID_RESOURCE_AGENDA);

	m_TabTotal.Create(WS_VISIBLE | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT /*| WS_VSCROLL */ | WS_TABSTOP, CRect(0, 0, 0, 0), &m_TabControlTasks, ID_RESOURCE_AGENDA_PAGE_1);
	m_TabTotal.SetFont(&m_FontTasks);
	m_TabTotal.ShowCategoryItemStates(true);

	m_TabMonthly.Create(WS_VISIBLE | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT /*| WS_VSCROLL */ | WS_TABSTOP, CRect(0, 0, 0, 0), &m_TabControlTasks, ID_RESOURCE_AGENDA_PAGE_2);
	m_TabMonthly.SetFont(&m_FontTasks);
	m_TabMonthly.ShowCategoryItemStates(true);

	// Add sys image list
	CImageList imagelistSys;
	CBitmap bmpSys;
	imagelistSys.Create(14, 14, ILC_COLOR24 | ILC_MASK, 4, 0);
	bmpSys.LoadBitmap(IDB_BITMAP_TABCTRL);
	imagelistSys.Add(&bmpSys, RGB(255, 0, 255));
	m_TabControlTasks.SetSystemImageList(&imagelistSys);

	// Add both tabs
	m_TabControlTasks.Add(m_TabTotal, _T("Todas"), -1);
	m_TabControlTasks.Add(m_TabMonthly, _T("Mensal"), -1);

	//Change style and layout
	m_TabControlTasks.InstallStyle(&style);

	m_TabControlTasks.SetLayout(TAB_LAYOUT_BOTTOM);
	m_TabControlTasks.SetBehavior(TAB_BEHAVIOR_SCROLL);
	
	//m_TabControlTasks.EqualTabsSize(true);

	m_TabControlTasks.ShowMenuButton(true);
	m_TabControlTasks.ShowScrollButtons(false);
	m_TabControlTasks.ShowCloseButton(false);

	LOGFONT logfont;
	m_TabControlTasks.GetFont()->GetLogFont(&logfont);
	logfont.lfWeight = FW_BOLD;
	m_TabControlTasks.SetFontSelect(&logfont);

	m_TabControlTasks.SetNotifyManager(this);
	m_TabControlTasks.Update();

	//m_Tasks.LoadXmlFromFile(_T("testando.xml"), CCatListBox::NoConvertAction);
	//m_Tasks.SaveXml(_T("testando.xml"), FALSE);
}

void CMainFrame::InitializeNotepad()
{
	//Create TabControl and its childs
	m_TabControlCustomPad.Create(this, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), ID_RESOURCE_NOTEPAD);

	// Add sys image list
	CImageList imagelistSys;
	CBitmap bmpSys;
	imagelistSys.Create(14, 14, ILC_COLOR24 | ILC_MASK, 4, 0);
	bmpSys.LoadBitmap(IDB_BITMAP_TABCTRL_NEW);
	imagelistSys.Add(&bmpSys, RGB(255, 0, 255));
	m_TabControlCustomPad.SetSystemImageList(&imagelistSys);

	// Add a new page
	CScintillaWnd *page1 = new CScintillaWnd();

	//Set Options for the Rich Edit Control 
	DWORD w_RichEd = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | /*WS_VSCROLL |*/ ES_AUTOVSCROLL | ES_MULTILINE;

	//Create Rich Edit Control to fill view
	if (!page1->Create(_T("tab"), w_RichEd, CRect(0, 0, 0, 0), &m_TabControlCustomPad, ID_RESOURCE_NOTEPAD_NEWPAGE))
		return;

	page1->Init();

	m_ListNotepad.push_back(page1);
	m_TabControlCustomPad.Add(*page1, _T("new 1*"), -1);

	//Change style and layout
	m_TabControlCustomPad.InstallStyle(&style);

	m_TabControlCustomPad.SetLayout(TAB_LAYOUT_BOTTOM);
	m_TabControlCustomPad.SetBehavior(TAB_BEHAVIOR_SCROLL);

	//m_TabControlTasks.EqualTabsSize(true);

	m_TabControlCustomPad.ShowMenuButton(false);
	m_TabControlCustomPad.ShowScrollButtons(false);
	m_TabControlCustomPad.ShowCloseButton(false);
	m_TabControlCustomPad.ShowAddTabButton(true);

	LOGFONT logfont;
	m_TabControlCustomPad.GetFont()->GetLogFont(&logfont);
	logfont.lfWeight = FW_BOLD;
	m_TabControlCustomPad.SetFontSelect(&logfont);

	m_TabControlCustomPad.SetNotifyManager(this);
	m_TabControlCustomPad.Update();

	/* Load Accelerator */
	m_hAccelNotepad = LoadAccelerators(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_TABCTRL_ACCELS));
}

void CMainFrame::InitializePinButton()
{
	CRect buttonRect(5, 5, 15, 12);
	m_PinButton.SetBitmapIDs(IDB_PUSHPIN_TEST);
	m_PinButton.Create(buttonRect, this, ID_RESOURCE_PINBUTTON);
}

void CMainFrame::EraseBackground(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);

	//rect.top = 400;
	pDC->FillSolidRect(&rect, palette[m_Palette][tone191]);

	// Draws background in degradè
	COLORREF tone,top,bot;				
	/*top = palette[m_Palette][tone239];
	bot = palette[m_Palette][tone191];*/

	top = RGB(242, 246, 251);
	bot = RGB(202, 218, 239);

	//rect.top = 0;
	//rect.bottom = 2;
	for (int i=0; i<200; i++) {
		tone = RGB(
			(GetRValue(bot)-GetRValue(top))*((float)i)/200.0f+GetRValue(top),
			(GetGValue(bot)-GetGValue(top))*((float)i)/200.0f+GetGValue(top),
			(GetBValue(bot)-GetBValue(top))*((float)i)/200.0f+GetBValue(top));
		pDC->FillSolidRect(rect,tone);
		rect.OffsetRect(0,2);
	}

	// Draw closing close
	CPen pen(PS_SOLID,1,palette[m_Palette][tone191]);
	pDC->SelectObject(pen);

	GetClientRect(&rect);
	rect.DeflateRect(rect.Width()-12,0,0,rect.Height()-12);
	rect.left -= 4;
	rect.right -= 4;
	rect.top += 3;
	rect.bottom += 4;

	CPoint poly[7];
	poly[0] = rect.TopLeft();
	poly[1] = rect.BottomRight();
	poly[2] = poly[1] + CSize( 0,-1);
	poly[3] = poly[0] + CSize( 1, 0);
	poly[4] = poly[3] + CSize(-1, 1);
	poly[5] = poly[1] + CSize(-1, 0);
	poly[6] = poly[1];
	pDC->Polyline(poly, 7);
	poly[0] = CPoint(rect.left, rect.bottom);
	poly[1] = CPoint(rect.right, rect.top);
	poly[2] = poly[1] + CSize( 0, 1);
	poly[3] = poly[0] + CSize( 1, 0);
	poly[4] = poly[3] + CSize(-1,-1);
	poly[5] = poly[1] + CSize(-1, 0);
	poly[6] = poly[1];
	pDC->Polyline(poly, 7);

	pDC->SelectStockObject(NULL_PEN);
}

void CMainFrame::DrawNotepadBkgnd(CDC *pDC)
{
	CFont *old = pDC->SelectObject(&m_FontTitle);

	COLORREF oldColor = pDC->SetTextColor(RGB(50, 50, 50));
	pDC->TextOut(25,581,"Anotações");

	CRect rect;	
	GetClientRect(&rect);
	//rect.DeflateRect(10,600,10,10);
	rect.DeflateRect(7, 600, 6, 10);

	CBrush *oldBrush = (CBrush *)pDC->SelectObject(&m_WhiteColor);
	pDC->RoundRect(&rect,CPoint(7,7));
	pDC->SelectObject(oldBrush);

	pDC->SelectObject(old);
	pDC->SelectObject(&oldColor);
}

void CMainFrame::DrawAgendaBkgnd(CDC *pDC)
{
	CFont *old = pDC->SelectObject(&m_FontTitle);

	COLORREF oldColor = pDC->SetTextColor(RGB(50, 50, 50));
	pDC->TextOut(38, 191, "Tarefas");

	CRect rect;
	GetClientRect(&rect);
	//rect.DeflateRect(10, 340, 10, 330);
	rect.DeflateRect(7, 210, 6, 0);
	rect.bottom = rect.top + SIZE_VALUE_TABCONTROLTASKS - 2;

	CBrush *oldBrush = (CBrush *)pDC->SelectObject(&m_WhiteColor);
	pDC->RoundRect(&rect, CPoint(7, 7));
	pDC->SelectObject(oldBrush);

	pDC->SelectObject(old);
	pDC->SelectObject(&oldColor);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	CRect rect;

	/* calendar */
	GetClientRect(&rect);

	//rect.DeflateRect(3, 180, 3, 0),
	rect.DeflateRect(3, 50, 3, 0),
		rect.bottom = rect.top + SIZE_VALUE_CALENDAR;

	m_Calendar.MoveWindow(&rect);

	/* notepad */
	GetClientRect(&rect);

	//rect.DeflateRect(10,600,10,10);
	rect.DeflateRect(4, 600, 4, 8);
	rect.DeflateRect(3, 3, 3, 3);

	//m_Notepad.MoveWindow(&rect);
	m_TabControlCustomPad.MoveWindow(&rect);

	/* Tab control tasks */
	GetClientRect(&rect);

	//rect.DeflateRect(7, 340, 8, 328);
	rect.DeflateRect(4, 210, 4, 0);
	rect.bottom = rect.top + SIZE_VALUE_TABCONTROLTASKS;
	rect.DeflateRect(3, 3, 3, 3);

	m_TabControlTasks.MoveWindow(&rect);

	/* pin button */
	GetClientRect(&rect);
	rect.DeflateRect(rect.Width() - 35, 0, 4, rect.Height() - 12);
	rect.left -= 4;
	rect.bottom -= 4;
	rect.top += 1;
	//rect.bottom += 5;

	m_PinButton.MoveWindow(&rect);

	/* calendar button */
	GetClientRect(&rect);
	int width = rect.Width();
	rect.DeflateRect(width - (21 * 2) - 20, 0, 0, rect.Height() - 21);
	rect.right = rect.left + 22;

	m_CalendarButton.MoveWindow(&rect);
}

void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect(&rect);
	
	//rect.DeflateRect(rect.Width()-12,4,4,rect.Height()-12);
	rect.DeflateRect(rect.Width() - 12, 0, 0, rect.Height() - 12);
	rect.left -= 4;
	rect.right -= 4;
	rect.top += 3;
	rect.bottom += 3;
	
	if (rect.PtInRect(point))
		::AfxPostQuitMessage(0);

	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CMainFrame::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	CFrameWnd::OnLButtonDblClk(nFlags, point);
}

BOOL CMainFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	/* tasks tree */
	/*CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(10, 340, 10, 330);
	rect.DeflateRect(3, 3, 3, 3);

	if (rect.PtInRect(pt))
	{
		m_Tasks.OnMouseWheel(nFlags, (zDelta > 0) ? 0 : 1, pt);
	}*/

	//m_Tasks.SetFocus();

	return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
}

// Actual handler
void CMainFrame::OnDynamicButtonClicked()
{
	m_isPinned = !m_isPinned;
	theApp.OnPinButton(m_isPinned);
}

/////////////////////////////////////////////////////////////////////////////
// TabCtrlNotify.
void CMainFrame::OnCloseButtonClicked(TabCtrl * /*pCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/)
{
	::MessageBox(m_hWnd, _T("CMainFrame::OnCloseButtonClicked"), _T("CMainFrame"), MB_OK);
}
void CMainFrame::OnMenuButtonClicked(TabCtrl * pCtrl, CRect const * /*pRect*/, CPoint ptScr)
{
	if (*pCtrl != m_TabControlTasks)
		return;

	CContextMenu ccmPopUp;
	ccmPopUp.CreatePopupMenu();

	// Customize the menu appearance and behavior
	ccmPopUp
		//.SetTextFont(CFont::FromHandle(&m_lf))
		//.SetColors(RGB(70, 36, 36), RGB(253, 249, 249), RGB(172, 96, 96), RGB(244, 234, 234), RGB(182, 109, 109));
		.SetColors(RGB(70, 36, 36), RGB(230, 240, 255), RGB(100, 96, 96), RGB(234, 234, 244), RGB(109, 109, 182));

	// Get a device context so that it'll be possible for the context menu
	// to calculate the size of the menu item's text
	CDC		*pDC = GetDC();
	int		iSaved = pDC->SaveDC();
	//CFont*	pOldFont = pDC->SelectObject(&m_Font);

	// ADDING MENU ITEMS - Start
	ccmPopUp.AppendMenuItem(MF_ENABLED, APPBAR_INSERTTASK, _T("Adicionar nova tarefa"), _T(""), pDC);

	ccmPopUp.AppendMenuItem(MF_SEPARATOR, 3, _T(""), _T(""), pDC);

	// Connecting lines related items
	ccmPopUp.AppendMenuItem(MF_ENABLED, APPBAR_LOADTASKTREE, _T("Importar"), _T(""), pDC);
	ccmPopUp.AppendMenuItem(MF_ENABLED, APPBAR_SAVETASKTREE, _T("Exportar"), _T(""), pDC);

	// ADDING MENU ITEMS - End

	bool alreadyPinned = false, dismissTemporaryPin = true;

	if (theApp.IsTemporaryPinned())
		alreadyPinned = true;
	else
		theApp.OnTemporaryPin(true);

	// Display the context menu
	UINT unCMD = ccmPopUp.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, ptScr.x, ptScr.y, this);

	switch (unCMD)
	{
		case APPBAR_INSERTTASK:
			OnInsertTask();
			break;
		case APPBAR_LOADTASKTREE:
			OnLoadAgenda();
			dismissTemporaryPin = false;
			break;
		case APPBAR_SAVETASKTREE:
			OnSaveAgenda();
			break;
		default:
			break;
	}

	if (!alreadyPinned && dismissTemporaryPin)
		theApp.OnTemporaryPin(false);

	// Clean up
	//pDC->SelectObject(pOldFont);
	pDC->RestoreDC(iSaved);
	ReleaseDC(pDC);
}

void CMainFrame::OnAddTabButtonClicked(TabCtrl * pCtrl, CRect const * pRect, CPoint ptScr)
{
	static int quantity = 1;

	// Add a new page
	CScintillaWnd *newPage = new CScintillaWnd();
	
	//Set Options for the Rich Edit Control 
	DWORD w_RichEd = WS_CHILD | WS_VISIBLE | /*WS_VSCROLL |*/ ES_AUTOVSCROLL | ES_MULTILINE;
	
	//Create Rich Edit Control to fill view
	if (!newPage->Create(_T("tab"), w_RichEd, CRect(0, 0, 0, 0), &m_TabControlCustomPad, ID_RESOURCE_NOTEPAD_NEWPAGE + quantity))
		return;
	
	newPage->Init();

	m_ListNotepad.push_back(newPage);

	CString tabName;
	tabName.Format("new %d*", quantity + 1);
	m_TabControlCustomPad.Add(*newPage,tabName, -1);
	
	m_TabControlCustomPad.ShowScrollButtons(true);
	
	// Set new tab as the current
	int lastTab = m_TabControlCustomPad.GetCount() - 1;
	HTAB tab = m_TabControlCustomPad.GetTab(lastTab);
	m_TabControlCustomPad.SetSel(tab);
	
	m_TabControlCustomPad.Update();
	m_TabControlCustomPad.ScrollToEnd();
	m_TabControlCustomPad.Update();

	quantity++;
}

void CMainFrame::OnDrag(TabCtrl *pCtrl, HTAB hTab, CPoint /*ptScr*/, bool outside)
{
	/*if (outside == true && m_bDelDragOutside == true)
	{
		pCtrl->Delete(hTab);
		pCtrl->Update();
	}*/
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Calendar Callback.
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnCalendarDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
	COleDateTime begin, end;
	m_Calendar.GetDateSel(begin, end);

	CInsertTaskDialog dialog(begin);
	
	theApp.OnTemporaryPin(true);
	int result = dialog.DoModal();

	if (result == IDOK)
	{

	}

	theApp.OnTemporaryPin(false);
}

LRESULT CMainFrame::OnCalendarHoveringToday(WPARAM wparam, LPARAM lparam)
{
	m_RectToday = (CRect*)wparam;
	
	CRect rect;
	m_Calendar.GetWindowRect(&rect);
	
	m_RectToday.top += rect.top;
	m_RectToday.left += rect.left;
	m_RectToday.bottom += rect.top;
	m_RectToday.right += rect.left;

	CSingleLock lock(m_pToolTipMutex);
	lock.Lock();

	if (!m_IsShowingTip)
	{		
		POINT p;
		if (GetCursorPos(&p))
		{
			//cursor position now in p.x and p.y
			//ScreenToClient(&p);
		
			//p.x and p.y are now relative to hwnd's client area
			CPoint point(p);

			SUPER_TOOLTIP_INFO sti;

			sti.bSuperTooltip = TRUE;
			sti.nVirtualKeyCode = 0;
			sti.nKeyCodeId = 0;
			sti.nIDTool = 0;
			sti.nSizeX = 220;
			sti.pWnd = (CWnd*)&m_Calendar;
			
			/* Header */
			COleDateTime date = COleDateTime::GetCurrentTime();
			CString strMonth = date.Format("%B");
			strMonth.SetAt(0, towupper(strMonth[0]));

			CString strHeader;
			strHeader.Format("Dia %d de %s", date.GetDay(), strMonth);

			sti.strHeader = strHeader;
			sti.bLineAfterHeader = FALSE;
			
			/* Body (Tasks) */
			if (m_TodayTaskList.size() > 0)
			{
				sti.strBody = "";

				std::vector<Task>::iterator it;
				for (it = m_TodayTaskList.begin(); it != m_TodayTaskList.end(); it++)
				{
					//sti.strBody += _T("Data: ") + it->oleDate.Format("%d/%m/%Y") + ("\r");

					if (it->iState == 1)
						sti.strBody += "<strike>";
					
					sti.strBody += it->strName;
					
					if (it->iState == 1)
						sti.strBody += "</strike>";

					sti.strBody += ("\n");
					//sti.strBody += _T("\n");
				}
			}
			else
			{
				COLORREF color = RGB(155, 0, 155);
				BYTE r = GetRValue(color);
				BYTE g = GetGValue(color);
				BYTE b = GetBValue(color);
				CString strFont = _T("");
				strFont.Format(_T("<font color=#%02X%02X%02X>"), r, g, b);
				
				sti.strBody	= strFont + _T("Não há nenhuma tarefa para hoje!") +_T("</font>\n");
			}
			
			/* Footer */
			sti.strFooter = _T("<a msg=\"F1\">Press F1 for more info.</a>");
			sti.nFooterImage = IDB_HELP;
			sti.bLineBeforeFooter = TRUE;
			
			/* Background color */
			sti.rgbBegin =	RGB(255, 255, 255);
			sti.rgbMid =	RGB(242, 246, 251);
			sti.rgbEnd =	RGB(202, 218, 239);
			sti.rgbText =	RGB(76, 76, 76);

			/* Rect bound to use it with mouse */
			sti.rectBounds = m_RectToday;

			CString strHtml = m_Tip.AddTool(&sti, &point);
			m_IsShowingTip = true;
		}
	}

	lock.Unlock();
	return 0;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Tab Control Task actions.
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnInsertTask()
{
	CInsertTaskDialog dialog;

	theApp.OnTemporaryPin(true);
	int result = dialog.DoModal();

	if (result == IDOK)
	{
		// category
		Task category;
		category.strCategory = dialog.GetTime().Format("%B/%y");
		category.strCategory.SetAt(0, towupper(category.strCategory[0]));

		bool alreadyExistCategory = false;
		for (std::vector<Task>::iterator it = m_ListTasks.begin(); it != m_ListTasks.end(); it++)
		{
			if (it->strCategory == category.strCategory && it->strName == "")
				alreadyExistCategory = true;
		}

		if (!alreadyExistCategory) 
		{
			category.strName = CString("");
			category.oleDate = COleDateTime();
			category.iState = -1;

			m_ListTasks.push_back(category);
		}

		// task item
		Task item;
		item.strCategory = category.strCategory;
		item.strName = dialog.GetTaskName();
		item.oleDate = dialog.GetTime();
		item.iState = -1;

		m_ListTasks.push_back(item);

		OnUpdateAgendaTabs();
	}

	theApp.OnTemporaryPin(false);
}


void CMainFrame::OnSaveAgenda()
{

}

void CMainFrame::OnLoadAgenda()
{
	CFileDialog dialog(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "XML Files(*.xml)|*.xml||", this);

	// Display the file dialog. When user clicks OK, fileDlg.DoModal() returns IDOK.
	if (dialog.DoModal() == IDOK)
	{
		CString strPathname = dialog.GetPathName();

		TRACE(_T("in CMainFrame::OnLoadAgenda: %s\n"), strPathname);
		//CTaskLoader::GetInstance().LoadXmlFromFile(strPathname, &m_TabTotal, CTaskLoader::NoConvertAction);
		
		//Clear all structures
		m_ListTasks.clear();
		m_TodayTaskList.clear();
		m_TabTotal.ResetContent();
		m_TabMonthly.ResetContent();

		BOOL result = CTaskLoader::GetInstance().LoadXmlFromFile(strPathname, &m_ListTasks, CTaskLoader::NoConvertAction);

		if (result == TRUE && m_ListTasks.size() > 0)
			OnUpdateAgendaTabs();

		theApp.OnTemporaryPin(true, 2000);
	}
}

void CMainFrame::OnUpdateAgendaTabs()
{
	//fill m_tabctrls page 1 and 2 
	std::vector<Task>::iterator it;

	COleDateTime date = COleDateTime::GetCurrentTime();
	CString strMonth = date.Format("%B/%y");
	strMonth.SetAt(0, towupper(strMonth[0]));

	TRACE("Month: %s\n", strMonth);
	
	bool foundInMonth = false;

	for (it = m_ListTasks.begin(); it != m_ListTasks.end(); it++)
	{
		if (it->strName.GetLength() == 0 && it->iState == -1)
		{
			it->iState == 0;
			m_TabTotal.AddCategory(it->strCategory);
		}
		else if(it->iState == -1)
		{
			it->iState = 0;
			CatListBoxItemInfo item;

			item.sItem = it->strName;
			item.sCategory = it->strCategory;
			item.iState = it->iState;
			m_TabTotal.AddCategoryItem(item);

			/* try to add to monthly */
			if (item.sCategory.Find(strMonth) >= 0)
			{
				m_TabMonthly.AddCategoryItem(item);
				foundInMonth = true;

				/* If it's possible add it to Today's task list */
				if (it->oleDate.GetDay() == date.GetDay())
				{
					m_TodayTaskList.push_back(*it);
				}
			}
		}
	}

	if (foundInMonth)
	{
		m_TabMonthly.SetCategoryState(strMonth, true);
		m_TabMonthly.UpdateData();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Tab Control Notepad actions.
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnTabSaveDoc()
{
	CWnd * wndActive = GetFocus();

	HTAB tab = m_TabControlCustomPad.GetSel();
	CWnd * wndTab = CWnd::FromHandle(m_TabControlCustomPad.GetTabWnd(tab));

	if (wndActive == wndTab)
	{
		CFileDialog dialog(FALSE, CString(".txt"), NULL, OFN_OVERWRITEPROMPT, "Normal text file (*.txt)|*.txt||", this);
		
		theApp.OnTemporaryPin(true);

		// Display the file dialog. When user clicks OK, fileDlg.DoModal() returns IDOK.
		if (dialog.DoModal() == IDOK)
		{
			CString strPathname = dialog.GetPathName();
			
			CScintillaWnd* pWnd = DYNAMIC_DOWNCAST(CScintillaWnd, wndTab);
			pWnd->SaveFile(strPathname);

			int charPos = strPathname.ReverseFind('\\') + 1;
			int totalLenghtWithoutExtension = strPathname.GetLength() - charPos - 4; // ".txt"
			CString strTabName = strPathname.Mid(charPos, totalLenghtWithoutExtension);

			m_TabControlCustomPad.SetTabText(tab, strTabName);
			m_TabControlCustomPad.Update();
		}
		theApp.OnTemporaryPin(false, 1000);

	}
	/*else
		TRACE("Foi mal, não deu igual muleke!\n");*/
}
