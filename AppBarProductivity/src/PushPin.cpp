/*
Module : PUSHPIN.H
Purpose: Implementation of a push pin button as seen on X-Windows & property dialogs in MS Developer Studio
Created: PJN / 04-04-1996
History: PJN / 08-06-1996 Removed win32sup.h include
         PJN / 20-11-1997 Major Update to code, changes include
                          1. Support non default color schemes
                          2. Supports dynamic changes to color schemes
                          3. Complete restructuring of code
                          4. Can now use an edged bitmap if you want to
                          5. Dropped support for Win16
                          6. Redid the example program 
                          7. Simplified external usage of class
         PJN / 24-11-1997 1. Minor changes to support CPushPinFrame class
         PJN / 07-12-1997 Minor changes to fix a small redraw bug
         PJN / 22-02-1999 1. A number of small code updates
         PJN / 02-06-1999 1. Added a number of comments to improvde code
                          readibility.
                          2. Changed the resouce id of the button bitmaps to more
                          meaningful values. 
                          3. Class no longer uses message reflection to
                          catch changes in the clicked state. This means that
                          parent classes can now catch the clicked notification.
                          4. Code now uses mouse capturing which means that the
                          button appears pressed if you click and hold the left
                          mouse button over the push pin. This gives the pushpin
                          class this standard button behaviour.
         PJN / 14-05-2000 Button now sends WM_COMMAND messages to the parent as a
                          standard button does.
         PJN / 07-03-2001 1. Updated copyright information   
                          2. Updated the pushpin frame and pushpin classes to behave exactly
                          like their counterparts in MS Developer Studio. Thanks to Abdulah DEDIC
                          for doing all the hard work.
         PJN / 27-08-2001 1. Fixed Access violation which was occuring in PreSubclassWindow on 
                          Windows 98.


Copyright (c) 1997 - 2001 by PJ Naughter.  (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "PushPin.h"




//////////////////////////////////  Macros  ///////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define WM_PPB_SIZE_TOCONTENT WM_USER+1



////////////////////////////////// Implementation /////////////////////////////
BEGIN_MESSAGE_MAP(CPushPinButton, CButton)
  //{{AFX_MSG_MAP(CPushPinButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_PPB_SIZE_TOCONTENT, OnSizeToContent)
END_MESSAGE_MAP()

CPushPinButton::CPushPinButton()
{
	//Initialise our variables to default values
	m_MaxRect = CRect(0, 0, 0, 0);
	m_bCaptured = FALSE;
  m_buttonState = UNPINNED_NORMAL;

	//Load up the bitmaps used in the push pin
	m_nButtons = 0;
	m_uiCombnBmpID = 0;
	m_bTrackLeave = FALSE;
	m_pWndMessageTo = NULL;
}

void CPushPinButton::ReloadBitmaps()
{
	//Reload the bitmaps
	LoadBitmaps();

	//size to content
	SizeToContent();

	//Invalidate the maximum rect of the pushpin on the parent window
	GetParent()->InvalidateRect(m_MaxRect);

	//Force this button to redraw aswell
	Invalidate();
}

void CPushPinButton::SetBitmapIDs(UINT uiCombnBmpID, BOOL b6Buttons, CWnd* pWndMessageTo)
{
	m_nButtons = b6Buttons ? MAX_BUTTON_INDEX : MAX_BUTTON_INDEX/2;
	m_uiCombnBmpID = uiCombnBmpID;
	m_pWndMessageTo = pWndMessageTo;
	LoadBitmaps();
}

BOOL CPushPinButton::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CButton::Create(_T(""), BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, rect, pParentWnd, nID))
		return FALSE;
	ReloadBitmaps();
	return TRUE;
}

void CPushPinButton::LoadBitmaps()
{
	//free the bitmap resources
	m_bmpCombined.DeleteObject();

	//Load up the necessary bitmaps using the SDK call LoadImage
	VERIFY(m_bmpCombined.Attach((HBITMAP)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(m_uiCombnBmpID),IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS)));
	BITMAP bitMap;
	VERIFY(m_bmpCombined.GetBitmap(&bitMap));
	ASSERT((bitMap.bmWidth % m_nButtons) == 0); //You're bitmap is not of the correct width
}

void CPushPinButton::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	ASSERT(lpDIS != NULL);

	//draw the bitmap into a compatible memory DC
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap* pOld = memDC.SelectObject(&m_bmpCombined);
	if (pOld == NULL)
		return;     // destructors will clean up

	//Then bitblt it onto the screen DC
	CRect rect;
	rect.CopyRect(&lpDIS->rcItem);
	BITMAP bitMap;
	VERIFY(m_bmpCombined.GetBitmap(&bitMap));
  int nButtonWidth = bitMap.bmWidth/m_nButtons;
	pDC->BitBlt(rect.left, rect.top, nButtonWidth, bitMap.bmHeight, &memDC, nButtonWidth*m_buttonState, 0, SRCCOPY);
	memDC.SelectObject(pOld);
} 

BOOL CPushPinButton::IsPinned() const
{
  return (m_buttonState == PINNED_NORMAL) || (m_buttonState == PINNED_DOWN) || (m_buttonState == PINNED_FLYBY);
}

void CPushPinButton::SetPinned(BOOL bPinned)
{
	if (m_nButtons < MAX_BUTTON_INDEX) //This button does not support pinning as
    return;                          //if does not have the correct number of bitmaps   

  if (bPinned)
    m_buttonState = PINNED_NORMAL;
  else
    m_buttonState = UNPINNED_NORMAL;

	Invalidate();
}

void CPushPinButton::PreSubclassWindow() 
{
  //Let the parent class do its thing
	CButton::PreSubclassWindow();

	//Pushpin buttons must be owner draw
	ASSERT(GetWindowLong(m_hWnd, GWL_STYLE) & BS_OWNERDRAW);

	//size to content (we need to use PostMessage here instead
  //of calling the function directly because on Win9x for some
  //stupid reason GetParent returns NULL at this point
  PostMessage(WM_PPB_SIZE_TOCONTENT);
}

LRESULT CPushPinButton::OnSizeToContent(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  //Just call the SizeToContent method
  SizeToContent();
  return 0L;
}

void CPushPinButton::SizeToContent()
{
	ASSERT(m_bmpCombined.m_hObject != NULL);

	BITMAP bmInfo;
	VERIFY(m_bmpCombined.GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo));
	CSize szBmp(bmInfo.bmWidth,bmInfo.bmHeight);
	if (m_uiCombnBmpID) 
    szBmp.cx /= m_nButtons;

	//Calculate the client rect in parent coordinates of the maximum size of the pushpin button
	m_MaxRect = CRect(0, 0, max(szBmp.cx, m_MaxRect.Width()), max(szBmp.cy, m_MaxRect.Height()));
	ClientToScreen(&m_MaxRect);

	CPoint p1(m_MaxRect.left, m_MaxRect.top);
	CPoint p2(m_MaxRect.right, m_MaxRect.bottom);

  CWnd* pParent = GetParent();
  ASSERT(pParent);
	pParent->ScreenToClient(&p1);
	pParent->ScreenToClient(&p2);
	m_MaxRect = CRect(p1, p2);

	//resize the button to match the size of the bitmap
	VERIFY(SetWindowPos(NULL, -1, -1, szBmp.cx, szBmp.cy, SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE));
}

void CPushPinButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//Capture the mouse
	SetCapture();
	m_bCaptured = TRUE;

  //Change the button state
  if (!IsPinned())
    m_buttonState = UNPINNED_DOWN;
	Invalidate();

	//Let the parent class do its thing
	CButton::OnLButtonDown(nFlags, point);
}

void CPushPinButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//Release the capture
	ReleaseCapture();
	m_bCaptured = FALSE;

	//Toggle the state of the pin and force a redraw
	//if over the button
	CRect r;									
	GetClientRect(&r);
  BOOL bInRect = r.PtInRect(point);

	if (bInRect)
	{
    //Update the UI
	  SetPinned(!IsPinned());

    //Inform the notification window by sending the 
    //WM_COMMAND message to the parents manually
		CWnd* pWndMessageTo = m_pWndMessageTo != NULL ? m_pWndMessageTo : GetParent();
		if (pWndMessageTo != NULL && pWndMessageTo->GetSafeHwnd() != NULL)
			pWndMessageTo->SendMessage(WM_COMMAND, MAKEWPARAM(GetWindowLong(GetSafeHwnd(), GWL_ID), 0), (LPARAM) GetSafeHwnd());
	}
  else
  {
    //Update the UI
    SetPinned(IsPinned());
  }

	//Let the parent class do its thing
	CButton::OnLButtonUp(nFlags, point);
}

void CPushPinButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect r;
	GetClientRect(&r);
	if (r.PtInRect(point))
  {
    if (IsPinned())
    {
      if (m_buttonState != PINNED_DOWN)
      {
        m_buttonState = PINNED_DOWN;
        Invalidate();
      }
    }
    else
    {
      if (!m_bCaptured)
      {
        if (m_buttonState != UNPINNED_FLYBY)
        {
          m_buttonState = UNPINNED_FLYBY;
          Invalidate();
        }
      }
      else
      {
        if (m_buttonState != UNPINNED_DOWN)
        {
          m_buttonState = UNPINNED_DOWN;
          Invalidate();
        }
      }
    }
  }
  else
  {
    if (!IsPinned())
    {
      if (m_buttonState != UNPINNED_FLYBY)
      {
        m_buttonState = UNPINNED_FLYBY;
        Invalidate();
      }
    }
  }

  //Ask that we be informed when the mouse leaves the button
	if (!m_bTrackLeave)
	{
		//First time mouse entered my window
		//request leave notification
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		_TrackMouseEvent(&tme);
		m_bTrackLeave = TRUE;
	}

	//Let the parent class do its thing		
	CButton::OnMouseMove(nFlags, point);
}

void CPushPinButton::OnCaptureChanged(CWnd* pWnd) 
{
	//Release the capture
	if (pWnd != this)
	{
		ReleaseCapture();
		m_bCaptured = FALSE;
		Invalidate();
	}

  //Let the parent class do its thing	
	CButton::OnCaptureChanged(pWnd);
}

LPARAM CPushPinButton::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  //Return the button to its non flyby state
  if (IsPinned())
    m_buttonState = PINNED_NORMAL;
  else
    m_buttonState = UNPINNED_NORMAL;
  m_bTrackLeave = FALSE;
  Invalidate();

	return 0L;
}

