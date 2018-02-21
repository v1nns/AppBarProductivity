/*
Module : PUSHPIN.H
Purpose: Interface of a push pin button (as seen on X-Windows & property dialogs in MS Developer Studio)
Created: PJN / 04-04-1996

Copyright (c) 1997 - 2001 by PJ Naughter.  
All rights reserved.

*/

#ifndef __PUSHPIN_H__
#define __PUSHPIN_H__


////////////////////////////////// Classes ///////////////////////////////////

class CPushPinButton : public CButton
{
public:
//Constructors / Destructors
	CPushPinButton();

//General Methods
	BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	void SetBitmapIDs(UINT uiCombnBmpID, BOOL b6Buttons = TRUE, CWnd* pWndMessageTo = NULL);

//Pinning support
	void SetPinned(BOOL bPinned);
	BOOL IsPinned() const;

//Should be called in response to system color changes
	void ReloadBitmaps(); 

protected:
//Enums
  enum ButtonState
  {
	  UNPINNED_NORMAL,
	  UNPINNED_FLYBY,
	  UNPINNED_DOWN,
	  PINNED_NORMAL,
	  PINNED_DOWN,
	  PINNED_FLYBY,
	  MAX_BUTTON_INDEX,
  };

  //{{AFX_MSG(CPushPinButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg LPARAM OnMouseLeave(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
  afx_msg LRESULT OnSizeToContent(WPARAM wParam, LPARAM lParam);

  //{{AFX_VIRTUAL(CPushPinButton)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	void SizeToContent();
	void LoadBitmaps();

	DECLARE_MESSAGE_MAP()
	CRect       m_MaxRect;
	BOOL        m_bCaptured;
  ButtonState m_buttonState;
	BOOL        m_bTrackLeave;
	CWnd*	      m_pWndMessageTo;
	UINT	      m_uiCombnBmpID;
	int		      m_nButtons;
	CBitmap     m_bmpCombined;
};

#endif //__PUSHPIN_H__