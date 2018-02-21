// CatListBox.cpp : Category Listbox implementation file.
//
// Author: Joshua Quick
//
// Features:
// -  Organizes listbox strings by categories.
// -  Categories can be opened to reveal their strings or closed to hide them.
// -  Category items can have checkboxes.
// -  Can store a DWORD with each category item.
//
// Do the following to add this control to a dialog:
// 1) Add a listbox to a dialog resource.
// 2) Setup listbox resource for "Owner Draw: Fixed" and check "Has Strings".
// 3) Create CCatListBox member variable in dialog by hand or wizard.
// *) If created by hand, then call <variable>.SubclassDlgItem(LISTBOX_ID,this).
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CatListBox.h"
#include "AppBarProductivity.h"

#pragma warning(disable:4018)
#pragma warning(disable:4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCatListBox::CCatListBox()
{
   m_mapCat.clear();
   m_bShowStates = false;
   m_bDoDraw     = true;
}

CCatListBox::~CCatListBox()
{
   ResetCategoryInfo();
}

//////////////////////////////////////////////////////////////////////
// Message mapping.
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CCatListBox, CListBox)
   //{{AFX_MSG_MAP(CCatListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// Message/event functions.
//////////////////////////////////////////////////////////////////////

// Draws strings into listbox.
void CCatListBox::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
   // Do not continue if draw flag is FALSE.
   // Works better than LockWindowUpdate() which prevents draws for all UI in app.
   if (!m_bDoDraw)
      return;

   // Validate argument.
   if (!lpDrawItemStruct || ((int)(lpDrawItemStruct->itemID) < 0))
      return;

   // Get list item's category info.
   CatListBoxCatInfo* pCatInfo = (CatListBoxCatInfo*)lpDrawItemStruct->itemData;
   if (!pCatInfo)
      return;

   // Setup device context.
   CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

   // Determine if list item is highlighted.
   bool bHighlight = ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
                      !(GetStyle() & LBS_NOSEL) && IsWindowEnabled());

   // Draw category's background.
   CRect    rcValue = lpDrawItemStruct->rcItem;
   COLORREF crNew   = GetSysColor( bHighlight ? COLOR_HIGHLIGHT : COLOR_WINDOW );
   COLORREF crOldBk = pDC->SetBkColor( crNew );
   CBrush   brWindow( crNew );
   pDC->FillRect( rcValue, &brWindow );

   // Determine position of category's button or item's checkbox.
   CRect rcButton( rcValue );
   rcButton.top    += Y_MARGIN;
   rcButton.left   += X_MARGIN;
   /*rcButton.bottom -= Y_MARGIN + 1;
   rcButton.right   = rcButton.left + rcButton.Height();*/
   rcButton.bottom = rcButton.top + CHECKBOX_SIZE;
   rcButton.right   = rcButton.left + CHECKBOX_SIZE;

   // Draw category/item.
   bool isThisItemACategory = false;
   if (lpDrawItemStruct->itemID == pCatInfo->iListIndex) {
      // Draw category's background.
      rcValue.bottom--;
      crNew = RGB(211,211,211);//GetSysColor( COLOR_3DFACE );
      if (!bHighlight) {
         pDC->SetBkColor( crNew );
         CBrush brBackGround( crNew );
         pDC->FillRect( rcValue, &brBackGround );
      }

      // Draw category's button frame.
      pDC->DrawFrameControl( rcButton, DFC_BUTTON, DFCS_BUTTONPUSH );

      // Draw "-" in button.
      crNew = GetSysColor( IsWindowEnabled() ? COLOR_BTNTEXT : COLOR_GRAYTEXT );
      int iX = rcButton.left + X_MARGIN;
      int iY = rcButton.top + (rcButton.Height() / 2);
      for (; iX < (rcButton.right - X_MARGIN); iX++)
         pDC->SetPixel( iX, iY, crNew );

      // Draw "|" in button if category is open, therefore forming a "+".
      if (!pCatInfo->bIsOpen) {
         iX = rcButton.left + (rcButton.Width() / 2);
         for (iY = rcButton.top + X_MARGIN; iY < (rcButton.bottom - X_MARGIN); iY++)
            pDC->SetPixel( iX, iY, crNew );
      }

	  isThisItemACategory = true;
   }
   else if (m_bShowStates) {
      // Draw item's checkbox according to its current state.
      CatListBoxStlListIter   iter;
      int                     iCount = (lpDrawItemStruct->itemID - pCatInfo->iListIndex) - 1;
      for (iter = pCatInfo->lstItems.begin(); iter != pCatInfo->lstItems.end(); iter++) {
         if (iCount <= 0) {
            if (0 == (*iter).iState)         // Draw a checked checkbox.
               pDC->DrawFrameControl( rcButton, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT );
            else if (1 == (*iter).iState)    // Draw an unchecked checkbox.
               pDC->DrawFrameControl( rcButton, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | DFCS_CHECKED );
            break;
         }
         iCount--;
      }
   }

   // Draw text.
   if (!IsWindowEnabled())
      crNew = GetSysColor( COLOR_GRAYTEXT );
   else if (bHighlight)
      crNew = GetSysColor( COLOR_HIGHLIGHTTEXT );
   else
      crNew = GetSysColor( COLOR_WINDOWTEXT );
   COLORREF crOldTxt = pDC->SetTextColor( crNew );
   CString  sText;
   GetText( lpDrawItemStruct->itemID, sText );
   int iTextPos = lpDrawItemStruct->rcItem.left;
   if (m_bShowStates || (lpDrawItemStruct->itemID == pCatInfo->iListIndex))
      iTextPos = rcButton.right;
   iTextPos += X_MARGIN;
   
   
   //pDC->TextOut( iTextPos, lpDrawItemStruct->rcItem.top + Y_MARGIN, sText );
   CRect rect;
   GetClientRect(&rect);
   CRect rc(lpDrawItemStruct->rcItem);
   
   rc.left = iTextPos;
   rc.top += Y_MARGIN;

   //pDC->DrawText (sText, &lpDrawItemStruct->rcItem, DT_LEFT | DT_WORDBREAK);
   CFont* pOldFont = NULL;
   if (isThisItemACategory)
   {
		//select a new font
	   CFont boldFont;
	   CFont* pFont = pDC->SelectObject(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
	   if (pFont)
	   {
		   LOGFONT logFont;
		   pFont->GetLogFont(&logFont);
		   logFont.lfHeight = 15;
		   logFont.lfWeight = FW_BOLD;//or FW_BOLD or FW_BLACK or other...
		   boldFont.CreateFontIndirect(&logFont);
		   pOldFont = pDC->SelectObject(&boldFont);
	   }
   }

	bool isChecked = false;
	if (!isThisItemACategory)
	{
		int index = GetCategoryItemIndex(lpDrawItemStruct->itemID);
		CString str = GetCategoryName(lpDrawItemStruct->itemID);
		if (GetCategoryItemState(str, index) == 1)
			isChecked = true;

		// If this item has been checked, then draw a line above it.
		if (IsWindowEnabled() && isChecked) 
		{
			crNew = GetSysColor(COLOR_GRAYTEXT);
			pDC->SetTextColor(crNew);

			int height = (rc.Height() / 3) + rc.top;
			CPoint p1(rc.left, height), p2(rc.right - 5, height);

			CBrush brFDiagonal;
			CBrush *pBrush;
			
			brFDiagonal.CreateHatchBrush(HS_FDIAGONAL, RGB(0, 128, 192));
			pBrush = pDC->SelectObject(&brFDiagonal);

			CPen brightwhitepen(PS_NULL, 1, RGB(255, 255, 255));
			CPen* pOldPen = pDC->SelectObject(&brightwhitepen);

			pDC->Rectangle(rc);

			//pDC->MoveTo(p1);
			//pDC->LineTo(p2);

			pDC->SelectObject(&pBrush);
			pDC->SelectObject(&pOldPen);
		}
	}

	int bkmode = pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(sText, sText.GetLength(), rc, DT_VCENTER | DT_WORDBREAK);
	pDC->SelectObject(&bkmode);

	if (pOldFont)
		pDC->SelectObject(pOldFont);

	// If this item has the focus, then draw a frame around it.
	if ((GetFocus() == this) && IsWindowEnabled()  &&
		(lpDrawItemStruct->itemState & ODS_FOCUS)) 
	{
		//pDC->DrawFocusRect( &(rc) );
	}

	// Restore DC's old settings.
	pDC->SetBkColor( crOldBk );
	pDC->SetTextColor( crOldTxt );
}

void CCatListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
 //   CClientDC   dc (this);        // handles GetDC and ReleaseDC internally
 //   CRect       rcText;
 //   CString     strText;

 //   // get text to be displayed, set initial rectangle to the values passed in
 //   // and calculate the necessary rectangle for the text
 //   GetText (lpMeasureItemStruct->itemID, strText);
 //   rcText.SetRect (0, 0, lpMeasureItemStruct->itemWidth, 
 //                   lpMeasureItemStruct->itemHeight);
 //   dc.DrawText (strText, -1, &rcText, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);

 //   // for some reason, CALCRECT returns a rectangle that is 16/13's the actual
 //   // necessary size.  13/16 = .8125, so we scale by .85 to allow a little room
 //   // below each item.  I chose not to set the width item and leave it at the 
 //   // default width
 //   lpMeasureItemStruct->itemHeight = (UINT)(rcText.Height () * 1.2);//0.85);
	////lpMeasureItemStruct->itemWidth = rcText.Width ();

	CFont *pFont = GetFont();
	CRect Rect;
	GetClientRect(&Rect);
	CDC *pDC = GetDC();
	CFont *pOldFont = pDC->SelectObject(pFont);
	CString Text;
	GetText(lpMeasureItemStruct->itemID,Text);

	if(m_bShowStates )
		Rect.left += CHECKBOX_SIZE;

	pDC->DrawText(Text,&Rect,DT_CALCRECT|DT_WORDBREAK);
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);
	//lpMeasureItemStruct->itemHeight = Rect.Height()+5;
	
	//workaround!
	int itemHeight = Rect.Height() + 5;
	if (itemHeight > 22)
		lpMeasureItemStruct->itemHeight = itemHeight;
	else
		lpMeasureItemStruct->itemHeight = 22;
}

// Opens/closes categories or checks/unchecks items.
void CCatListBox::OnLButtonDown( UINT nFlags, CPoint point )
{
   CatListBoxCatInfo*   pCatInfo;
   CString              sCategory;
   CRect                rcValue;
   int                  iHeight;
   int                  iListIndex;

   // Get listbox's height.
   GetWindowRect( rcValue );
   iHeight = rcValue.Height();

   // Determine if a category button has been clicked.
   for (iListIndex = GetTopIndex(); iListIndex < GetCount(); iListIndex++) {
      // Get item's category info.
      pCatInfo = (CatListBoxCatInfo*)GetItemData( iListIndex );
      if (!pCatInfo)
         continue;

      // Calculate button position.
      GetItemRect( iListIndex, rcValue );
      rcValue.top    += Y_MARGIN;
      rcValue.bottom -= Y_MARGIN + 1;
      rcValue.left   += X_MARGIN;
      rcValue.right   = rcValue.left + rcValue.Height();

      // Stop now if reached lower displayable bound of listbox.
      if (rcValue.top >= iHeight)
         break;

      // Determine if item's button was clicked.
      if (rcValue.PtInRect( point )) {
         GetText( pCatInfo->iListIndex, sCategory );
         SetCurSel( iListIndex );
         if (iListIndex == pCatInfo->iListIndex) {
            // Open/close category.
            SetCategoryState( (LPCTSTR)sCategory, !(pCatInfo->bIsOpen) );
         }
         else if (m_bShowStates) {
            // Check/uncheck item's checkbox if it has a checkbox.
            int iItemIndex = (iListIndex - pCatInfo->iListIndex) - 1;
            int iState = GetCategoryItemState( (LPCTSTR)sCategory, iItemIndex );
            if (0 == iState)
               SetCategoryItemState( (LPCTSTR)sCategory, iItemIndex, 1 );
            else if (1 == iState)
               SetCategoryItemState( (LPCTSTR)sCategory, iItemIndex, 0 );
         }
         break;      // Done. State has been updated.
      }
   }
   CListBox::OnLButtonDown( nFlags, point );
}

// Opens/closes categories.
void CCatListBox::OnLButtonDblClk( UINT nFlags, CPoint point )
{
   CatListBoxCatInfo*   pCatInfo;
   CString              sCategory;
   CRect                rcValue;
   int                  iHeight;
   int                  iListIndex;

   // Get listbox's height.
   GetWindowRect( rcValue );
   iHeight = rcValue.Height();

   // Determine if a category has been double clicked.
   for (iListIndex = GetTopIndex(); iListIndex < GetCount(); iListIndex++) {
      // Get listbox item's category info.
      pCatInfo = (CatListBoxCatInfo*)GetItemData( iListIndex );
      if (!pCatInfo)
         continue;

      // Skip item if it's not a category.
      if (iListIndex != pCatInfo->iListIndex)
         continue;

      // Determine if category has been double clicked.
      GetItemRect( iListIndex, rcValue );
      if (rcValue.top >= iHeight)
         break;         // Stop searching. Lower listbox bound has been reached.
      if (rcValue.PtInRect( point )) {
         GetText( iListIndex, sCategory );
         SetCurSel( iListIndex );
         SetCategoryState( (LPCTSTR)sCategory, !(pCatInfo->bIsOpen) );
         break;         // Done. Category has been opened/closed.
      }
   }
   CListBox::OnLButtonDblClk( nFlags, point );
}

// Opens/closes categories or checks/unchecks items when the space bar is pressed.
void CCatListBox::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
   if (VK_SPACE == nChar) {
      int iListIndex = GetCurSel();
      if (iListIndex != LB_ERR) {
         CatListBoxCatInfo* pInfo = (CatListBoxCatInfo*)GetItemData( iListIndex );
         if (pInfo && (LB_ERR != (int)pInfo)) {
            CString  sCategory;
            GetText( pInfo->iListIndex, sCategory );
            if (iListIndex == pInfo->iListIndex) {
               // Open/close category.
               SetCategoryState( (LPCTSTR)sCategory, !(pInfo->bIsOpen) );
            }
            else if (m_bShowStates) {
               // Check/uncheck item if it has a checkbox.
               int iItemIndex = (iListIndex - pInfo->iListIndex) - 1;
               int iState = GetCategoryItemState( (LPCTSTR)sCategory, iItemIndex );
               if (0 == iState)
                  SetCategoryItemState( (LPCTSTR)sCategory, iItemIndex, 1 );
               else if (1 == iState)
                  SetCategoryItemState( (LPCTSTR)sCategory, iItemIndex, 0 );
            }
         }
      }
   }
   else if (VK_RIGHT == nChar || VK_LEFT == nChar) {
	   int iListIndex = GetCurSel();
	   if (iListIndex != LB_ERR) {
		   CatListBoxCatInfo* pInfo = (CatListBoxCatInfo*)GetItemData(iListIndex);
		   if (pInfo && (LB_ERR != (int)pInfo)) {
			   CString  sCategory;
			   GetText(pInfo->iListIndex, sCategory);
			   if (iListIndex == pInfo->iListIndex) {
				   // Open/close category.
				   bool open = (nChar == VK_RIGHT ? true : false);
				   SetCategoryState((LPCTSTR)sCategory, open);
			   }
		   }
	   }
   }
   else
	   CListBox::OnKeyDown( nChar, nRepCnt, nFlags );
}

//=============================================================================
void CCatListBox::OnRButtonDown(UINT nFlags, CPoint point)
//=============================================================================
{
	/* Get the cursor position for this message */ 
	DWORD dwPos = GetMessagePos();
	
	/* Convert the co-ords into a CPoint structure */ 
	CPoint pt(GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos));
	CPoint spt = pt;

	/* convert to screen co-ords for the hittesting to work */
	ScreenToClient(&spt);
	BOOL bOutside;
	UINT index = ItemFromPoint(spt, bOutside);

	/* Is it inside the listbox area? */
	if (!bOutside)
	{
		CRect listBoxRect;
		GetItemRect(index, &listBoxRect);

		/* Is it inside the listbox item area? */
		if(PtInRect(listBoxRect, spt))
			SetCurSel(index);
		else
			SetCurSel(-1);
	}
	else
		SetCurSel(-1);

	CListBox::OnRButtonDown(nFlags, point);
}

//=============================================================================
void CCatListBox::OnContextMenu(CWnd* pWnd, CPoint point)
//=============================================================================
{
	//do nothing at this time
}

//////////////////////////////////////////////////////////////////////
// OnContextMenu actions
//////////////////////////////////////////////////////////////////////
void CCatListBox::OnDeleteNode(int indexItemSelected)
{

}

void CCatListBox::OnModifyNodeText(int indexItemSelected)
{

}

//////////////////////////////////////////////////////////////////////
// Add functions.
//////////////////////////////////////////////////////////////////////

// Adds a new category to listbox. (Category name must be unique!)
HRESULT CCatListBox::AddCategory( LPCTSTR pCategory )
{
   CatListBoxStlMapIter    iter;
   CatListBoxCatInfo*      pCatInfo;
   CatListBoxCatInfo*      pTempInfo;

   // Validate argument.
   if (!pCategory)
      return E_POINTER;
   if (_tcslen( pCategory ) <= 0)
      return E_INVALIDARG;
   if (m_mapCat.end() != m_mapCat.find(CString(pCategory))) // Category name must be unique!
      return E_INVALIDARG;

   // Add category to STL map.
   try {
      pCatInfo = new CatListBoxCatInfo;
      if (!pCatInfo)
         return E_FAIL;
      pCatInfo->bIsOpen    = false;
      pCatInfo->iListIndex = -1;
      m_mapCat.insert( CatListBoxStlMap::value_type( CString(pCategory), pCatInfo ) );
   }
   catch (...) {
      ASSERT(0);
      return E_FAIL;
   }

   // Add category to listbox.
   if (LBS_SORT & GetStyle()) {
      for (iter = m_mapCat.begin(); iter != m_mapCat.end(); iter++) {
         pTempInfo = (*iter).second;
         if (!pTempInfo || (pTempInfo == pCatInfo))
            continue;
         if ((*iter).first.CompareNoCase( pCategory ) > 0) {
            if ((UINT)pCatInfo->iListIndex > (UINT)pTempInfo->iListIndex)
               pCatInfo->iListIndex = pTempInfo->iListIndex;
            pTempInfo->iListIndex++;      // Increment index of all following categories.
         }
      }
   }
   m_bDoDraw = false;      // Disables listbox drawing.
   pCatInfo->iListIndex = InsertString( pCatInfo->iListIndex, pCategory );
   SetItemData( pCatInfo->iListIndex, (DWORD)pCatInfo );
   m_bDoDraw = true;       // Enables listbox drawing. Must wait for SetItemData() to be called.
   Invalidate();

   return S_OK;
}

int CCatListBox::AddCategoryItem(CatListBoxItemInfo item)
{
	return AddCategoryItem(item.sCategory, item.sItem, item.iState);
}

// Adds an item under the specified category. Creates category if it hasn't already been added.
// Also sets the item's state. 0 = Unchecked checkbox. 1 = Checked checkbox. 2 = No checkbox.
// Returns the category item index to the new string or LB_ERR if the add failed.
int CCatListBox::AddCategoryItem( LPCTSTR pCategory, LPCTSTR pItem, int iState /*=2*/ )
{
   CatListBoxCatInfo*      pCatInfo;
   CatListBoxItemInfo      ItemInfo;
   CatListBoxStlListIter   iter;
   int                     iItemIndex = 0;
   int                     iListIndex;

   // Validate argument.
   if (!pCategory)
      return LB_ERR;

   // Get category
   pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo) {
      // Category not found. Add it to map.
      if (S_OK != AddCategory( pCategory ))
         return LB_ERR;
      pCatInfo = GetCategoryInfo( pCategory );
      if (!pCatInfo)
         return LB_ERR;
   }

   // Add string to category's STL list.
   if (LBS_SORT & GetStyle()) {
      iItemIndex = 0;
      for (iter = pCatInfo->lstItems.begin(); iter != pCatInfo->lstItems.end(); iter++) {
         if ((*iter).sItem.CompareNoCase( pItem ) > 0)
            break;
         iItemIndex++;
      }
   }
   else {
      iter       = pCatInfo->lstItems.end();
      iItemIndex = pCatInfo->lstItems.size();
   }
   ItemInfo.dwValue = 0;
   ItemInfo.iState  = iState;
   ItemInfo.sItem   = pItem;
   pCatInfo->lstItems.insert( iter, ItemInfo );

   // Add string to listbox if its category is open.
   if (pCatInfo->bIsOpen) {
      // Add string.
      m_bDoDraw = false;      // Disables listbox drawing.
      iListIndex = pCatInfo->iListIndex + iItemIndex + 1;
      iListIndex = InsertString( iListIndex, pItem );
      if (iListIndex >= 0) {
         SetItemData( iListIndex, (DWORD)pCatInfo );
         UpdateCategoryIndexes( pCatInfo->iListIndex, 1 );
      }
      m_bDoDraw = true;       // Enables listbox drawing. Wait for SetItemData() to be called.
      Invalidate();
   }

   return iItemIndex;
}

// Overloaded and made protected to prevent this function from being called from outside.
// Must use AddCategoryItem(pCategory,pItem) instead.
int CCatListBox::AddString( LPCTSTR pString )
{
   return CListBox::AddString( pString );
}

// Overloaded and made protected to prevent this function from being called from outside.
int CCatListBox::InsertString( int iListIndex, LPCTSTR pString )
{
   return CListBox::InsertString( iListIndex, pString );
}


//////////////////////////////////////////////////////////////////////
// Delete functions.
//////////////////////////////////////////////////////////////////////

// Deletes the indexed listbox item.
// If the index points to a category's item, then only that item is deleted.
// If the index points to a category, then that category and all of its items are deleted.
HRESULT CCatListBox::Delete( int iListIndex )
{
   CatListBoxCatInfo*   pInfo;
   CString              sCategory;
   HRESULT              hr = E_FAIL;

   // Validate argument.
   if ((iListIndex < 0) || (iListIndex >= GetCount()))
      return E_INVALIDARG;

   // Get category's info.
   pInfo = (CatListBoxCatInfo*)GetItemData( iListIndex );
   if (!pInfo)
      return E_FAIL;

   // Delete indexed listbox item.
   GetText( pInfo->iListIndex, sCategory );
   if (iListIndex == pInfo->iListIndex)
      hr = DeleteCategory( (LPCTSTR)sCategory );
   else
      hr = DeleteCategoryItem( (LPCTSTR)sCategory, (iListIndex - pInfo->iListIndex) - 1 );
   return hr;
}

// Deletes specified category and all of its items.
HRESULT CCatListBox::DeleteCategory( LPCTSTR pCategory )
{
   CatListBoxCatInfo*      pInfo;
   CatListBoxStlMapIter    iter;
   int                     iCount = 0;

   // Validate argument.
   if (!pCategory)
      return E_POINTER;

   // Get category's info.
   iter = m_mapCat.find( CString( pCategory ) );
   if (iter == m_mapCat.end())
      return E_INVALIDARG;
   pInfo = (*iter).second;
   if (!pInfo)
      return E_INVALIDARG;

   // Delete category and its items from listbox.
   m_bDoDraw = false;      // Disables listbox drawing.
   while ((CatListBoxCatInfo*)GetItemData( pInfo->iListIndex ) == pInfo) {
      DeleteString( pInfo->iListIndex );
      iCount++;
   }
   UpdateCategoryIndexes( pInfo->iListIndex, 0 - iCount );
   m_bDoDraw = true;       // Enables listbox drawing. Clears listbox strings all at once.
   Invalidate();

   // Delete category from STL map.
   delete pInfo;
   (*iter).second = NULL;
   m_mapCat.erase( iter );
   return S_OK;
}

// Deletes indexed category item.
HRESULT CCatListBox::DeleteCategoryItem( LPCTSTR pCategory, int iItemIndex )
{
   CatListBoxCatInfo*      pInfo;
   CatListBoxStlListIter   iter;
   int                     iValue;

   // Validate argument.
   if (!pCategory)
      return E_POINTER;

   // Get category's info.
   pInfo = GetCategoryInfo( pCategory );
   if (!pInfo)
      return E_INVALIDARG;

   // Do not continue if "iItemIndex" exceeds list's bounds.
   if ((iItemIndex < 0) || (iItemIndex >= pInfo->lstItems.size()))
      return E_INVALIDARG;

   // Delete string in listbox if its category is open.
   if (pInfo->bIsOpen) {
      m_bDoDraw = false;   // Disables listbox drawing.
      iValue = pInfo->iListIndex + iItemIndex + 1;
      DeleteString( iValue );
      UpdateCategoryIndexes( iValue, -1 );
      m_bDoDraw = true;    // Enables listbox drawing. Wait for category index update.
      Invalidate();
   }

   // Find item in category's list.
   // Delete item from category's list.
   iValue = 0;
   for (iter = pInfo->lstItems.begin(); iter != pInfo->lstItems.end(); iter++) {
      if (iValue == iItemIndex)
         break;
      iValue++;
   }
   pInfo->lstItems.erase( iter );
   return S_OK;
}

// Overloaded and made protected to prevent this function from being called from outside.
// Must use DeleteCategoryItem(pCategory,pItem) instead.
int CCatListBox::DeleteString( UINT iListIndex )
{
   return CListBox::DeleteString( iListIndex );
}


//////////////////////////////////////////////////////////////////////
// Reset functions.
//////////////////////////////////////////////////////////////////////

// Empties listbox.
void CCatListBox::ResetContent()
{
   // Empty listbox.
   CListBox::ResetContent();

   // Empty STL map.
   ResetCategoryInfo();
}

// Empties category STL map.
void CCatListBox::ResetCategoryInfo()
{
   CatListBoxStlMapIter    iter;
   CatListBoxCatInfo*      pCatInfo;

   for (iter = m_mapCat.begin(); iter != m_mapCat.end(); iter++) {
      if (pCatInfo = (*iter).second) {
         delete pCatInfo;
         (*iter).second = NULL;
      }
   }
   m_mapCat.clear();
}


//////////////////////////////////////////////////////////////////////
// Find functions.
//////////////////////////////////////////////////////////////////////

// Returns specified category's listbox index or -1 if not found.
int CCatListBox::FindCategory( LPCTSTR pCategory )
{
   CatListBoxCatInfo* pInfo = GetCategoryInfo( pCategory );
   return (pInfo) ? pInfo->iListIndex : -1;
}

// Finds the first item string exactly matching "pItem" in the category's list.
// Will search after index "iStartAfter" in list. Set "iStartAfter" to -1 to search entire list.
// Returns the category item's index or -1 if not found. (Does not return listbox index!)
int CCatListBox::FindCategoryItem( LPCTSTR pCategory, int iStartAfter, LPCTSTR pItem )
{
   CatListBoxCatInfo*      pInfo;
   CatListBoxStlListIter   iter;
   int                     iCount = 0;

   // Validate arguments.
   if (!pCategory || !pItem)
      return -1;

   // Get category's info.
   pInfo = GetCategoryInfo( pCategory );
   if (!pInfo)
      return -1;

   // Find item in category's list.
   for (iter = pInfo->lstItems.begin(); iter != pInfo->lstItems.end(); iter++) {
      if (iCount > iStartAfter) {
         if (0 == (*iter).sItem.Compare( pItem )) {
            return iCount;
         }
      }
      iCount++;
   }
   return -1;
}


//////////////////////////////////////////////////////////////////////
// Is functions.
//////////////////////////////////////////////////////////////////////

// Returns TRUE if listbox index references a category.
bool CCatListBox::IsCategory( int iListIndex )
{
   CatListBoxCatInfo* pInfo = (CatListBoxCatInfo*)GetItemData( iListIndex );
   if (pInfo && (LB_ERR != (int)pInfo)) {
      if (iListIndex == pInfo->iListIndex) {
         return true;
      }
   }
   return false;
}

// Returns TRUE if specified string is a category name.
bool CCatListBox::IsCategory( LPCTSTR pString )
{
   return (FindCategory( pString ) >= 0) ? true : false;
}

// Returns TRUE if the specified category is open.
// Returns FALSE if closed or if category does not exist.
bool CCatListBox::IsCategoryOpen( LPCTSTR pCategory )
{
   CatListBoxCatInfo* pInfo = GetCategoryInfo( pCategory );
   if (pInfo) {
      if (pInfo->bIsOpen) {
         return true;
      }
   }
   return false;
}


//////////////////////////////////////////////////////////////////////
// Get functions.
//////////////////////////////////////////////////////////////////////

// Returns the number of categories stored in this listbox.
int CCatListBox::GetCategoryCount()
{
   return m_mapCat.size();
}

// Returns the category's name specified by the given index.
// If "bIsListIndex" = TRUE, then "iIndex" references the listbox.
// If "bIsListIndex" = FALSE, then "iIndex" references the category list.
// (The total number of categories can be retrieved via GetCategoryCount().)
LPCTSTR CCatListBox::GetCategoryName( int iIndex, bool bIsListIndex /*=true*/ )
{
   CatListBoxStlMapIter    iter;
   CatListBoxCatInfo*      pInfo;

   // Find category name.
   if (bIsListIndex) {
      // Get category name by listbox index.
      pInfo = (CatListBoxCatInfo*)GetItemData( iIndex );
      if (pInfo && (LB_ERR != (int)pInfo)) {
         for (iter = m_mapCat.begin(); iter != m_mapCat.end(); iter++) {
            if (pInfo == (*iter).second) {
               return (LPCTSTR)((*iter).first);
            }
         }
      }
   }
   else {
      // Get category name by category index.
      if ((iIndex >= 0) && (iIndex < m_mapCat.size())) {
         for (iter = m_mapCat.begin(); iter != m_mapCat.end(); iter++) {
            if (iIndex <= 0)
               return (LPCTSTR)((*iter).first);
            iIndex--;
         }
      }
   }
   return NULL;
}

// Returns the number of items under the specified category.
int CCatListBox::GetCategoryItemCount( LPCTSTR pCategory )
{
   CatListBoxCatInfo* pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo)
      return -1;
   return pCatInfo->lstItems.size();
}

// Returns data stored with the specified category's item. Returns LB_ERR if not found.
DWORD CCatListBox::GetCategoryItemData( LPCTSTR pCategory, int iItemIndex )
{
   CatListBoxCatInfo*      pCatInfo;
   CatListBoxStlListIter   iter;

   // Validate arguments.
   if (!pCategory || (iItemIndex < 0))
      return LB_ERR;

   // Get category's structure.
   pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo)
      return LB_ERR;

   // Get DWORD at specified index.
   for (iter = pCatInfo->lstItems.begin(); iter != pCatInfo->lstItems.end(); iter++) {
      if (iItemIndex <= 0)
         return (*iter).dwValue;
      iItemIndex--;
   }
   return LB_ERR;
}

// Overloaded and made protected to prevent this function from being called from outside.
// Must use GetCategoryItemData(pCategory,iItemIndex) instead.
DWORD CCatListBox::GetItemData( int iListIndex )
{
   return CListBox::GetItemData( iListIndex );
}

// Returns the category item index matching the specified listbox index.
int CCatListBox::GetCategoryItemIndex( int iListIndex )
{
   CatListBoxCatInfo* pInfo = (CatListBoxCatInfo*)GetItemData( iListIndex );
   if (pInfo && (LB_ERR != (int)pInfo))
      return (iListIndex - pInfo->iListIndex) - 1;
   return -1;
}

// Returns the category item name by category item index. Returns NULL if not found.
// This is the only way to retrieve an item name while its category is closed.
LPCTSTR CCatListBox::GetCategoryItemName( LPCTSTR pCategory, int iItemIndex )
{
   CatListBoxCatInfo*      pInfo;
   CatListBoxStlListIter   iter;

   // Get category info.
   pInfo = GetCategoryInfo( pCategory );
   if (!pInfo)
      return NULL;

   // Get item name.
   for (iter = pInfo->lstItems.begin(); iter != pInfo->lstItems.end(); iter++) {
      if (iItemIndex <= 0)
         return (LPCTSTR)((*iter).sItem);
      iItemIndex--;
   }
   return NULL;
}

// Returns the state value stored by the specified category's item. Returns LB_ERR on error.
// State 0 means unchecked. State 1 means checked. State 2 means has no check state.
int CCatListBox::GetCategoryItemState( LPCTSTR pCategory, int iItemIndex )
{
   CatListBoxCatInfo*      pInfo;
   CatListBoxStlListIter   iter;

   // Get category info.
   pInfo = GetCategoryInfo( pCategory );
   if (!pInfo)
      return LB_ERR;

   // Validate "iItemIndex" bounds.
   if ((iItemIndex < 0) || (iItemIndex >= pInfo->lstItems.size()))
      return E_INVALIDARG;

   // Get item name.
   for (iter = pInfo->lstItems.begin(); iter != pInfo->lstItems.end(); iter++) {
      if (iItemIndex <= 0)
         return (*iter).iState;
      iItemIndex--;
   }
   return LB_ERR;
}

// Returns info structure owned by the specified category or NULL if category was not found.
CatListBoxCatInfo* CCatListBox::GetCategoryInfo( LPCTSTR pCategory )
{
   CatListBoxStlMapIter iter;

   // Validate argument.
   if (!pCategory)
      return NULL;

   // Get category's STL list.
   iter = m_mapCat.find( CString((LPCSTR)pCategory) );
   if (iter == m_mapCat.end())
      return NULL;
   return (*iter).second;
}


//////////////////////////////////////////////////////////////////////
// Set functions.
//////////////////////////////////////////////////////////////////////

// Stores "dwValue" with the specified category item.
HRESULT CCatListBox::SetCategoryItemData( LPCTSTR pCategory, int iItemIndex, DWORD dwValue )
{
   CatListBoxCatInfo*      pCatInfo;
   CatListBoxStlListIter   iter;

   // Validate arguments.
   if (!pCategory)
      return E_POINTER;

   // Get category info.
   pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo)
      return E_INVALIDARG;

   // Validate "iItemIndex" bounds.
   if ((iItemIndex < 0) || (iItemIndex >= pCatInfo->lstItems.size()))
      return E_INVALIDARG;

   // Store "dwValue".
   for (iter = pCatInfo->lstItems.begin(); iter != pCatInfo->lstItems.end(); iter++) {
      if (iItemIndex <= 0) {
         (*iter).dwValue = dwValue;
         break;
      }
      iItemIndex--;
   }
   return S_OK;
}

// Overloaded and made protected to prevent this function from being called from outside.
// Must use SetCategoryItemData(pCategory,iItemIndex,dwValue) instead.
int CCatListBox::SetItemData( int iListIndex, DWORD dwValue )
{
   return CListBox::SetItemData( iListIndex, dwValue );
}

// Selects item under the specified category. Expands category if currently closed.
HRESULT CCatListBox::SetCurSel( LPCTSTR pCategory, int iItemIndex )
{
   CatListBoxCatInfo*   pCatInfo;

   // Validate.
   if (!pCategory)
      return E_POINTER;

   // Get category info.
   pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo)
      return E_INVALIDARG;

   // Validate "iItemIndex" bounds.
   if ((iItemIndex < 0) || (iItemIndex >= pCatInfo->lstItems.size()))
      return E_INVALIDARG;

   // Expand category if it is currently closed.
   if (!pCatInfo->bIsOpen)
      SetCategoryState( pCategory, true );

   // Select string.
   SetCurSel( pCatInfo->iListIndex + iItemIndex + 1 );
   return S_OK;
}

// Selects the specified listbox item by listbox index.
int CCatListBox::SetCurSel( int iListIndex )
{
   return CListBox::SetCurSel( iListIndex );
}

// Sets the category's state in the listbox.
// "bOpen" = TRUE expands the category and displays its strings in listbox.
// "bOpen" = FALSE closes category and hides its strings in listbox.
HRESULT CCatListBox::SetCategoryState( LPCTSTR pCategory, bool bOpen )
{
   CatListBoxCatInfo*      pCatInfo;
   CatListBoxStlListIter   iter;
   CString                 sBuf;
   int                     iListIndex;

   // Get category info.
   pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo)
      return E_INVALIDARG;

   // Do not continue if state hasn't changed.
   if (pCatInfo->bIsOpen == bOpen)
      return S_OK;

   // Change state.
   pCatInfo->bIsOpen = bOpen;
   iListIndex = pCatInfo->iListIndex;
   m_bDoDraw = false;         // Disables listbox drawing.
   if (bOpen) {
      // Open category. Insert its items into listbox.
      for (iter = pCatInfo->lstItems.begin(); iter != pCatInfo->lstItems.end(); iter++) {
         iListIndex = (iListIndex >= (GetCount() - 1)) ? -1 : iListIndex + 1;
         iListIndex = InsertString( iListIndex, (*iter).sItem );
         SetItemData( iListIndex, (DWORD)pCatInfo );
      }
      UpdateCategoryIndexes( pCatInfo->iListIndex, iListIndex - pCatInfo->iListIndex );
   }
   else {
      // Close category. Delete its items from listbox.
      iListIndex++;
      while ((CatListBoxCatInfo*)GetItemData( iListIndex ) == pCatInfo)
         DeleteString( iListIndex );
      UpdateCategoryIndexes( pCatInfo->iListIndex, 0 - pCatInfo->lstItems.size() );
   }
   m_bDoDraw = true;          // Enables listbox drawing. Shows/hides new strings all at once.
   Invalidate();
   return S_OK;
}

// Sets the category item's state in the listbox.
// "iState" = 0 unchecks its checkbox.
// "iState" = 1 checks its checkbox.
// "iState" = 2 shows no checkbox. (Has no check state.)
HRESULT CCatListBox::SetCategoryItemState( LPCTSTR pCategory, int iItemIndex, int iState )
{
   CatListBoxCatInfo*      pCatInfo;
   CatListBoxStlListIter   iter;

   // Get category info.
   pCatInfo = GetCategoryInfo( pCategory );
   if (!pCatInfo)
      return E_INVALIDARG;

   // Validate "iItemIndex" bounds.
   if ((iItemIndex < 0) || (iItemIndex >= pCatInfo->lstItems.size()))
      return E_INVALIDARG;

   // Find item in category's list.
   for (iter = pCatInfo->lstItems.begin(); iter != pCatInfo->lstItems.end(); iter++) {
      if (iItemIndex <= 0) {
         // Found it. Set item's state.
         (*iter).iState = iState;
         if (m_bShowStates)
            Invalidate();
         return S_OK;
      }
      iItemIndex--;
   }
   return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
// Show functions.
//////////////////////////////////////////////////////////////////////

// Shows/hides all category item checkboxes. Returns the last show state.
// (Hiding will shift all category item text to the left near the listbox border.)
bool CCatListBox::ShowCategoryItemStates( bool bShow /*=true*/ )
{
   bool  bValue = m_bShowStates;
   m_bShowStates = bShow;
   Invalidate();
   return bValue;
}


//////////////////////////////////////////////////////////////////////
// Update functions.
//////////////////////////////////////////////////////////////////////

// Changes all category listbox indexes by "iOffset", starting after listbox index "iStartAfter".
// This function should be called after inserts, deletes, and category open/close.
void CCatListBox::UpdateCategoryIndexes( int iStartAfter, int iOffset )
{
   CatListBoxStlMapIter    iter;
   CatListBoxCatInfo*      pCatInfo;

   // Do not continue if offset is zero. Wouldn't change anything.
   if (!iOffset)
      return;

   // Update category listbox indexes.
   for (iter = m_mapCat.begin(); iter != m_mapCat.end(); iter++) {
      pCatInfo = (*iter).second;
      if (!pCatInfo)
         continue;
      if (pCatInfo->iListIndex <= iStartAfter)
         continue;
      pCatInfo->iListIndex += iOffset;
   }
}

BOOL CCatListBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (iDeltaPerLine == 0)
		return FALSE;

	iAccumDelta += zDelta;     // 120 or -120
	while (iAccumDelta >= iDeltaPerLine)
	{
		SendMessage(WM_VSCROLL, SB_LINEUP, 0);
		iAccumDelta -= iDeltaPerLine;
	}

	while (iAccumDelta <= -iDeltaPerLine)
	{
		SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
		iAccumDelta += iDeltaPerLine;
	}

	return FALSE;
	//return CListBox::OnMouseWheel(nFlags, zDelta, pt);
}


void CCatListBox::PreSubclassWindow()
{
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);

	// ulScrollLines usually equals 3 or 0 (for no scrolling)
	// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
	if (ulScrollLines)
		iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
	else
		iDeltaPerLine = 0;

	iAccumDelta = 0;

	CListBox::PreSubclassWindow();
}
