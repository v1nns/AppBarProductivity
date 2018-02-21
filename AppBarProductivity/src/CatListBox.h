// CatListBox.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CATLISTBOX_H__9C65B2F7_70D4_11D6_923D_00C04F04DB80__INCLUDED_)
#define AFX_CATLISTBOX_H__9C65B2F7_70D4_11D6_923D_00C04F04DB80__INCLUDED_

// Disable compiler warning C4786.
// This warning is caused by STL map's class name which is longer than 255 characters.
// Very long class names work fine, but they cannot be viewed in the debugger.
#pragma warning(disable:4786)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>
#include <map>
using namespace std;

#include "XHtmlTree\XmlDocument.h"

#define CHECKBOX_SIZE 15

#define		APPBAR_DELETENODE				WM_APP + 10010
#define		APPBAR_MODIFYNODETEXT			WM_APP + 10011

// Define STLs.
struct CatListBoxItemInfo {
   CString     sItem;
   CString     sCategory;
   int         iState;
   DWORD       dwValue;
};

typedef list<CatListBoxItemInfo>          CatListBoxStlList;
typedef CatListBoxStlList::iterator       CatListBoxStlListIter;
struct CatListBoxCatInfo {
   bool                 bIsOpen;
   int                  iListIndex;
   CatListBoxStlList    lstItems;
};
typedef map<CString,CatListBoxCatInfo*>   CatListBoxStlMap;
typedef CatListBoxStlMap::iterator        CatListBoxStlMapIter;


// Category listbox class.
class CCatListBox : public CListBox
{
public:
   // Constructor/Destructor.
   CCatListBox();
   virtual ~CCatListBox();

   HRESULT  AddCategory( LPCTSTR pCategory );
   int      AddCategoryItem(CatListBoxItemInfo item);
   int      AddCategoryItem( LPCTSTR pCategory, LPCTSTR pItem, int iState = 2 );
   HRESULT  Delete( int iListIndex );
   HRESULT  DeleteCategory( LPCTSTR pCategory );
   HRESULT  DeleteCategoryItem( LPCTSTR pCategory, int iItemIndex );
   int      FindCategory( LPCTSTR pCategory );
   int      FindCategoryItem( LPCTSTR pCategory, int iStartAfter, LPCTSTR pItem );
   bool     IsCategory( int iListIndex );
   bool     IsCategory( LPCTSTR pString );
   bool     IsCategoryOpen( LPCTSTR pCategory );
   int      GetCategoryCount();
   LPCTSTR  GetCategoryName( int iIndex, bool bIsListIndex = true );
   int      GetCategoryItemCount( LPCTSTR pCategory );
   DWORD    GetCategoryItemData( LPCTSTR pCategory, int iItemIndex );
   int      GetCategoryItemIndex( int iListIndex );
   LPCTSTR  GetCategoryItemName( LPCTSTR pCategory, int iItemIndex );
   int      GetCategoryItemState( LPCTSTR pCategory, int iItemIndex );
   HRESULT  SetCurSel( LPCTSTR pCategory, int iItemIndex );
   int      SetCurSel( int iListIndex );
   HRESULT  SetCategoryItemData( LPCTSTR pCategory, int iItemIndex, DWORD dwValue );
   HRESULT  SetCategoryItemState( LPCTSTR pCategory, int iItemIndex, int iState );
   HRESULT  SetCategoryState( LPCTSTR pCategory, bool bOpen );
   bool     ShowCategoryItemStates( bool bShow = true );
   void     ResetContent();

   // Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CCatListBox)
private:
   virtual void PreSubclassWindow();

   virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
   virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
   //}}AFX_VIRTUAL

protected:
   // Generated message map functions
   //{{AFX_MSG(CCatListBox)
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

   afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

   afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

   /* Related to actions on OnContextMenu */
   void OnDeleteNode(int indexItemSelected);
   void OnModifyNodeText(int indexItemSelected);
   
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()

   CatListBoxCatInfo*   GetCategoryInfo( LPCTSTR pCategory );
   DWORD                GetItemData( int iListIndex );
   int                  SetItemData( int iListIndex, DWORD dwValue );
   int                  AddString( LPCTSTR pString );
   int                  InsertString( int iListIndex, LPCTSTR pString );
   int                  DeleteString( UINT iListIndex );
   void                 ResetCategoryInfo();
   void                 UpdateCategoryIndexes( int iStartAfter, int iOffset );

//=============================================================================
//	Private member variables.
//=============================================================================
private:
   CatListBoxStlMap     m_mapCat;
   bool                 m_bShowStates;
   bool                 m_bDoDraw;

   // Protected constants.
   enum { X_MARGIN = 3, Y_MARGIN = 3 };

   // Mouse wheel
   int  iDeltaPerLine, iAccumDelta;     // for mouse wheel logic
   ULONG ulScrollLines;                 // for mouse wheel logic
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CATLISTBOX_H__9C65B2F7_70D4_11D6_923D_00C04F04DB80__INCLUDED_)
