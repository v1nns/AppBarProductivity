#pragma once

#include "XHtmlTree\XmlDocument.h"

/* in order to use its own structures */
#include "CatListBox.h"

/* task structure */
struct Task
{
	CString strName;
	CString strCategory;
	COleDateTime oleDate;

	int iState;
};

class CTaskLoader
{
// Constructor/Destructor
private:
	CTaskLoader();
	virtual ~CTaskLoader();

// Singleton
public:
	static CTaskLoader& GetInstance();
	static void ReleaseResources();

//=============================================================================
// XML functions
//=============================================================================
public:
	enum ConvertAction
	{
		NoConvertAction = 0, ConvertToUnicode, ConvertToAnsi
	};

	BOOL		ConvertBuffer(const BYTE * inbuf,
		DWORD inlen,				// in bytes
		BYTE ** outbuf,
		DWORD& outlen,			// in bytes
		ConvertAction eConvertAction = NoConvertAction);

//=============================================================================
// Related to CatListBox
//=============================================================================
public:
	BOOL		LoadXmlFromBuffer(const BYTE * pBuf, DWORD len, CCatListBox* pList, ConvertAction eConvertAction);
	BOOL		LoadXmlFromFile(LPCTSTR lpszFile, CCatListBox* pList, ConvertAction eConvertAction);

	BOOL		SaveXml(LPCTSTR lpszFileName, BOOL bSaveAsUTF16, CCatListBox* pList);

protected:
	HRESULT		InsertXmlItem(CXmlElement *pElement, CCatListBox* pList, bool showCheckbox = TRUE);
	BOOL		LoadXml(CXmlDocument& xml, CXmlElement *pElement, int& nCount, CCatListBox* pList);

	CString		GetXmlText(CatListBoxCatInfo hCat, const CString& strCategoryName);
	CString		GetXmlText(CatListBoxItemInfo hItem);

//=============================================================================
// Related to Task
//=============================================================================
public:
	BOOL		LoadXmlFromBuffer(const BYTE * pBuf, DWORD len, std::vector<Task>* pList, ConvertAction eConvertAction);
	BOOL		LoadXmlFromFile(LPCTSTR lpszFile, std::vector<Task>* pList, ConvertAction eConvertAction);

protected:
	HRESULT		InsertXmlItem(CXmlElement *pElement, std::vector<Task>* pList, bool showCheckbox = TRUE);
	BOOL		LoadXml(CXmlDocument& xml, CXmlElement *pElement, int& nCount, std::vector<Task>* pList);

//	CString		GetXmlText(CatListBoxCatInfo hCat, const CString& strCategoryName);
//	CString		GetXmlText(CatListBoxItemInfo hItem);

//=============================================================================
// Class Variables
//=============================================================================
private:
	int			m_nXmlCount;			// count of items loaded from XML file
	CString		m_strCurrentCategory;	// used to link the item to a category

	static CTaskLoader* m_pInstance; // Singleton
};

