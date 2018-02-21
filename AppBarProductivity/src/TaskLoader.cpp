#include "stdafx.h"
#include "TaskLoader.h"

#pragma warning(disable:4018)
#pragma warning(disable:4996)

// ---- inicialização estática --------
CTaskLoader* CTaskLoader::m_pInstance = NULL;

/* Singleton para obter a instancia corrente*/
CTaskLoader& CTaskLoader::GetInstance()
{
	if (!m_pInstance)
	{
		m_pInstance = new CTaskLoader();
	}
	return *m_pInstance;
}

void CTaskLoader::ReleaseResources()
{
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

/* private constructor */
CTaskLoader::CTaskLoader()
{
}

/* private destructor */
CTaskLoader::~CTaskLoader()
{
}

//////////////////////////////////////////////////////////////////////
// XML Structure
//////////////////////////////////////////////////////////////////////
//=============================================================================
BOOL CTaskLoader::ConvertBuffer(const BYTE * inbuf,
	DWORD inlen,				// in bytes
	BYTE ** outbuf,
	DWORD& outlen,			// in bytes
	ConvertAction eConvertAction /*= NoConvertAction*/)
	//=============================================================================
{
	BOOL rc = FALSE;

	ASSERT(inbuf);
	ASSERT(inlen != 0);

	outlen = 0;

	if (inbuf && (inlen != 0))
	{
		TRACE(_T("eConvertAction=%d\n"), eConvertAction);

		// copy buffer to ensure it's null terminated
		BYTE * buf = new BYTE[inlen + 16];
		memset(buf, 0, inlen + 16);
		memcpy(buf, inbuf, inlen);

		if (eConvertAction == ConvertToUnicode)
		{
			TRACE(_T("converting to unicode\n"));
			int wlen = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buf, -1, NULL, 0);
			TRACE(_T("wlen=%d\n"), wlen);
			WCHAR *pszText = new WCHAR[wlen + 16];
			memset(pszText, 0, (wlen + 16) * sizeof(WCHAR));
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buf, -1, pszText, wlen + 2);
			outlen = (DWORD)wcslen(pszText) * sizeof(WCHAR);	// bytes
			*outbuf = (BYTE*)pszText;
			delete[] buf;
		}
		else if (eConvertAction == ConvertToAnsi)
		{
			TRACE(_T("converting to ansi\n"));
			LPCWSTR wp = (LPCWSTR)buf;
			int alen = WideCharToMultiByte(CP_ACP, 0, wp, -1, NULL, 0, NULL, NULL);
			TRACE(_T("alen=%d\n"), alen);
			char *pszText = new char[alen + 16];
			memset(pszText, 0, alen + 16);
			WideCharToMultiByte(CP_ACP, 0, wp, -1, pszText, alen + 2, NULL, NULL);
			outlen = (DWORD)strlen(pszText);	// bytes
			*outbuf = (BYTE*)pszText;
			delete[] buf;
		}
		else
		{
			// no conversion
			outlen = inlen;						// bytes
			*outbuf = buf;
		}

		TRACE(_T("outlen=%d\n"), outlen);

		rc = TRUE;
	}

	return rc;
}

/////////////////////////////////////////////////////////////////////////////
// Related to CCatListBox
/////////////////////////////////////////////////////////////////////////////
BOOL CTaskLoader::LoadXmlFromFile(LPCTSTR lpszFile, CCatListBox* pList, ConvertAction eConvertAction)
{
	TRACE(_T("in CCatListBox::LoadXmlFromFile: %s\n"), lpszFile);

	BOOL rc = FALSE;

	DWORD dwFileSize = 0;
	HANDLE hFile = NULL;

	ASSERT(lpszFile);
	ASSERT(pList);

	if (lpszFile && pList)
	{
		hFile = CreateFile(lpszFile,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			TRACE(_T("ERROR - CreateFile failed\n"));
			hFile = 0;
		}
		else
		{
			dwFileSize = GetFileSize(hFile, NULL);

			if ((dwFileSize != INVALID_FILE_SIZE))
			{
				BYTE * pBuf = new BYTE[dwFileSize + 16];
				ASSERT(pBuf);

				if (pBuf)
				{
					DWORD dwBytesRead = 0;
					BOOL bRet = ReadFile(hFile,
						(LPVOID)pBuf,
						dwFileSize,
						&dwBytesRead,
						NULL);

					if (bRet)
					{
						pBuf[dwFileSize] = 0;
						pBuf[dwFileSize + 1] = 0;
						pBuf[dwFileSize + 2] = 0;

						LoadXmlFromBuffer(pBuf, dwFileSize, pList,
							eConvertAction);

						rc = TRUE;
					}
					else
					{
						TRACE(_T("ERROR - ReadFile failed\n"));
						dwFileSize = 0;
					}

					delete[] pBuf;
				}
				else
				{
					TRACE(_T("ERROR - failed to allocate memory\n"));
					dwFileSize = 0;
				}
			}
			else
			{
				TRACE(_T("ERROR - GetFileSize failed\n"));
				dwFileSize = 0;
			}
		}
	}
	else
	{
		TRACE(_T("ERROR - bad parameter\n"));
	}

	if (hFile)
		CloseHandle(hFile);

	return rc;
}

//=============================================================================
BOOL CTaskLoader::LoadXmlFromBuffer(const BYTE * pBuf,
	DWORD len,	// bytes
	CCatListBox* pList,
	ConvertAction eConvertAction)
//=============================================================================
{
	BOOL rc = FALSE;

	ASSERT(pBuf);

	if (pBuf)
	{
		DWORD outlen = 0;
		BYTE * outbuf = 0;

		rc = ConvertBuffer(pBuf, len, &outbuf, outlen,
			eConvertAction);

		if (rc)
		{
			CString strXML = (TCHAR *)outbuf;

			delete[] outbuf;

			// remove endofline and tabs
			strXML.Remove(_T('\n'));
			strXML.Remove(_T('\r'));
			strXML.Remove(_T('\t'));

			m_nXmlCount = 0;

			CXmlDocument xml;
			BOOL ret = xml.Parse(strXML);
			TRACE(_T("Parse returned %d\n"), ret);

			if (ret)		// useless, always returns TRUE
			{
				/*TRACE(_T("loading root +++++\n"));
				HTREEITEM hRoot = InsertXmlItem(xml.GetRootElement(), 0, false);
				if (hRoot)
				{
				//m_phRoot = &hRoot;
				m_nXmlCount++;
				}*/

				TRACE(_T("loading children +++++\n"));
				LoadXml(xml, xml.GetRootElement(), m_nXmlCount, pList);

				rc = TRUE;
			}
			else
			{
				TRACE(_T("ERROR - Parse failed\n"));
			}
		}
	}

	return rc;
}

//=============================================================================
BOOL CTaskLoader::LoadXml(CXmlDocument& xml,
	CXmlElement *pElement,
	int& nCount,
	CCatListBox* pList)
//=============================================================================
{
	BOOL rc = TRUE;

	ASSERT(pElement);
	ASSERT(pList);

	if (pElement && pList)
	{
		CXmlElement *pChild = xml.GetFirstChild(pElement);

		while (pChild != NULL)
		{
			//Check if this is a category item
			CString strIsCategory;
			strIsCategory = pChild->GetValue(_T("category"));
			if (!strIsCategory.IsEmpty())
			{
				m_strCurrentCategory = pChild->GetValue(_T("name"));
				TRACE(_T("LoadXml: <%s>\n"), m_strCurrentCategory);
			}

			HRESULT result = InsertXmlItem(pChild, pList);

			if (result != S_OK)
			{
				// found '...'
				rc = FALSE;
				break;
			}

			nCount++;

			rc = LoadXml(xml, pChild, nCount, pList);
			if (!rc)
				break;

			pChild = xml.GetNextSibling(pElement);
		}
	}

	return rc;
}

//=============================================================================
HRESULT CTaskLoader::InsertXmlItem(CXmlElement *pElement, CCatListBox* pList, bool showCheckbox)
//=============================================================================
{
	TRACE(_T("in CTaskLoader::InsertXmlItem\n"));
	HRESULT result = LB_ERR;

	ASSERT(pElement);
	ASSERT(pList);

	if (pElement && pList)
	{
		CString strName = pElement->GetValue(_T("name"));
		TRACE(_T("InsertXmlItem: <%s>\n"), strName);

		// hack to stop display
		if (/*!strName.IsEmpty() &&*/ strName.Compare(_T("...")) != 0)
		{
			strName.TrimLeft();
			strName.TrimRight();

#if 0  // -----------------------------------------------------------
			if (strName.GetLength() <= 0)
			{
				TRACE(_T("ERROR - name is empty\n"));
				ASSERT(FALSE);
			}
#endif // -----------------------------------------------------------

			CString strIsCategory = pElement->GetValue(_T("category"));

			if (!strIsCategory.IsEmpty())
				result = pList->AddCategory(strName);
			else
			{
				CatListBoxItemInfo item;

				item.sCategory = m_strCurrentCategory;

				CString strName = pElement->GetValue(_T("name"));
				if (!strName.IsEmpty())
					item.sItem = strName;

				CString strChecked = pElement->GetValue(_T("checked"));
				if (!strChecked.IsEmpty())
					item.iState = _ttoi(strChecked);

				/*CString strEnabled = pElement->GetValue(_T("enabled"));
				if (!strEnabled.IsEmpty() && !item.bSeparator)
				item.bEnabled = _ttoi(strEnabled);*/

				TRACE(_T("category=<%s>, name=<%s>, <%s> \n"), m_strCurrentCategory, strName, strChecked);
				if (pList->AddCategoryItem(item) >= 0)
					result = S_OK;
				else
					result = LB_ERR;
			}
		}
	}

	return result;
}

BOOL CTaskLoader::SaveXml(LPCTSTR lpszFile, BOOL bSaveAsUTF16, CCatListBox* pList)	// useful only on VS2005
{
	BOOL rc = FALSE;

	ASSERT(lpszFile);
	ASSERT(pList);

	if (lpszFile && pList)
	{
		FILE *f = NULL;

#if _MSC_VER >= 1400
		if (bSaveAsUTF16)
			f = _tfopen(lpszFile, _T("w, ccs=UTF-16LE"));
		else
#else
		UNUSED_ALWAYS(bSaveAsUTF16);
#endif
		f = _tfopen(lpszFile, _T("w"));

		if (f && pList->GetCategoryCount() > 0)
		{
			_ftprintf(f, _T("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n"));

			CString ws = _T("");
			CString strText = _T("");

			CatListBoxStlMapIter    iter;
			CatListBoxStlListIter   iterList;

			CatListBoxCatInfo*      pCatInfo;

			strText.Format(_T("<root name=\"Tarefas\">"));
			_ftprintf(f, strText);

			int iListIndex;
			for (iListIndex = pList->GetTopIndex(); iListIndex < pList->GetCount(); iListIndex++)
			{
				CString strCategory = pList->GetCategoryName(iListIndex);

				// Get item's category info.
				pCatInfo = (CatListBoxCatInfo*)pList->GetCategoryItemData(strCategory, iListIndex);

				// Is it a category??
				if (pCatInfo)
				{
					if (iListIndex != pList->GetTopIndex())
					{
						strText.Format(_T("\n\t</category>\n"));
						_ftprintf(f, strText);
					}

					strText = GetXmlText(*pCatInfo, pList->GetCategoryName(iListIndex));
					_ftprintf(f, strText);

					// iterate through tasks
					for (iterList = pCatInfo->lstItems.begin(); iterList != pCatInfo->lstItems.end(); iterList++)
					{
						strText = GetXmlText(*iterList);
						_ftprintf(f, strText);
					}
				}
			}

			strText.Format(_T("\n\t</category>"));
			_ftprintf(f, strText);
		}

		_ftprintf(f, _T("\n</root>"));
		fflush(f);
		fclose(f);
		rc = TRUE;
	}

	return rc;
}

//=============================================================================
CString CTaskLoader::GetXmlText(CatListBoxCatInfo hCat, const CString& strCategoryName)
//=============================================================================
{
	CString strElem = _T("category");
	CString s = strCategoryName;

	CString strXml = _T("");
	strXml.Format(_T("\n\t<%s name=\"%s\""), strElem, s);

	s.Format(_T(" category=\"%s\""), _T("1"));
	strXml += s;

	s.Format(_T(" checked=\"%s\""), hCat.bIsOpen ? _T("1") : _T("0"));
	strXml += s;

	s.Format(_T(" enabled=\"%s\">"), _T("1"));
	strXml += s;

	return strXml;
}

//=============================================================================
CString CTaskLoader::GetXmlText(CatListBoxItemInfo hItem)
//=============================================================================
{
	CString strElem = _T("task");
	CString s = hItem.sItem;

	CString strXml = _T("");
	strXml.Format(_T("\n\t\t<%s name=\"%s\""), strElem, s);

	s.Format(_T(" checked=\"%s\""), hItem.iState ? _T("1") : _T("0"));
	strXml += s;

	s.Format(_T(" enabled=\"%s\">"), _T("1"));
	strXml += s;

	//\n\t\t
	s.Format(_T("</%s>"), strElem);
	strXml += s;

	return strXml;
}

/////////////////////////////////////////////////////////////////////////////
// Related to Task
/////////////////////////////////////////////////////////////////////////////
BOOL CTaskLoader::LoadXmlFromFile(LPCTSTR lpszFile, std::vector<Task>* pList, ConvertAction eConvertAction)
{
	TRACE(_T("in CCatListBox::LoadXmlFromFile: %s\n"), lpszFile);

	BOOL rc = FALSE;

	DWORD dwFileSize = 0;
	HANDLE hFile = NULL;

	ASSERT(lpszFile);
	ASSERT(pList);

	if (lpszFile && pList)
	{
		hFile = CreateFile(lpszFile,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			TRACE(_T("ERROR - CreateFile failed\n"));
			hFile = 0;
		}
		else
		{
			dwFileSize = GetFileSize(hFile, NULL);

			if ((dwFileSize != INVALID_FILE_SIZE))
			{
				BYTE * pBuf = new BYTE[dwFileSize + 16];
				ASSERT(pBuf);

				if (pBuf)
				{
					DWORD dwBytesRead = 0;
					BOOL bRet = ReadFile(hFile,
						(LPVOID)pBuf,
						dwFileSize,
						&dwBytesRead,
						NULL);

					if (bRet)
					{
						pBuf[dwFileSize] = 0;
						pBuf[dwFileSize + 1] = 0;
						pBuf[dwFileSize + 2] = 0;

						LoadXmlFromBuffer(pBuf, dwFileSize, pList,
							eConvertAction);

						rc = TRUE;
					}
					else
					{
						TRACE(_T("ERROR - ReadFile failed\n"));
						dwFileSize = 0;
					}

					delete[] pBuf;
				}
				else
				{
					TRACE(_T("ERROR - failed to allocate memory\n"));
					dwFileSize = 0;
				}
			}
			else
			{
				TRACE(_T("ERROR - GetFileSize failed\n"));
				dwFileSize = 0;
			}
		}
	}
	else
	{
		TRACE(_T("ERROR - bad parameter\n"));
	}

	if (hFile)
		CloseHandle(hFile);

	return rc;
}

//=============================================================================
BOOL CTaskLoader::LoadXmlFromBuffer(const BYTE * pBuf,
	DWORD len,	// bytes
	std::vector<Task>* pList,
	ConvertAction eConvertAction)
	//=============================================================================
{
	BOOL rc = FALSE;

	ASSERT(pBuf);

	if (pBuf)
	{
		DWORD outlen = 0;
		BYTE * outbuf = 0;

		rc = ConvertBuffer(pBuf, len, &outbuf, outlen,
			eConvertAction);

		if (rc)
		{
			CString strXML = (TCHAR *)outbuf;

			delete[] outbuf;

			// remove endofline and tabs
			strXML.Remove(_T('\n'));
			strXML.Remove(_T('\r'));
			strXML.Remove(_T('\t'));

			m_nXmlCount = 0;

			CXmlDocument xml;
			BOOL ret = xml.Parse(strXML);
			TRACE(_T("Parse returned %d\n"), ret);

			if (ret)		// useless, always returns TRUE
			{
				/*TRACE(_T("loading root +++++\n"));
				HTREEITEM hRoot = InsertXmlItem(xml.GetRootElement(), 0, false);
				if (hRoot)
				{
				//m_phRoot = &hRoot;
				m_nXmlCount++;
				}*/

				TRACE(_T("loading children +++++\n"));
				LoadXml(xml, xml.GetRootElement(), m_nXmlCount, pList);

				rc = TRUE;
			}
			else
			{
				TRACE(_T("ERROR - Parse failed\n"));
			}
		}
	}

	return rc;
}

//=============================================================================
BOOL CTaskLoader::LoadXml(CXmlDocument& xml,
	CXmlElement *pElement,
	int& nCount,
	std::vector<Task>* pList)
	//=============================================================================
{
	BOOL rc = TRUE;

	ASSERT(pElement);
	ASSERT(pList);

	if (pElement && pList)
	{
		CXmlElement *pChild = xml.GetFirstChild(pElement);

		while (pChild != NULL)
		{
			//Check if this is a category item
			CString strIsCategory;
			strIsCategory = pChild->GetValue(_T("category"));
			if (!strIsCategory.IsEmpty())
			{
				m_strCurrentCategory = pChild->GetValue(_T("name"));
				TRACE(_T("LoadXml: <%s>\n"), m_strCurrentCategory);
			}

			HRESULT result = InsertXmlItem(pChild, pList);

			if (result != S_OK)
			{
				// found '...'
				rc = FALSE;
				break;
			}

			nCount++;

			rc = LoadXml(xml, pChild, nCount, pList);
			if (!rc)
				break;

			pChild = xml.GetNextSibling(pElement);
		}
	}

	return rc;
}

//=============================================================================
HRESULT CTaskLoader::InsertXmlItem(CXmlElement *pElement, std::vector<Task>* pList, bool showCheckbox)
//=============================================================================
{
	TRACE(_T("in CTaskLoader::InsertXmlItem\n"));
	HRESULT result = LB_ERR;

	ASSERT(pElement);
	ASSERT(pList);

	if (pElement && pList)
	{
		CString strName = pElement->GetValue(_T("name"));
		TRACE(_T("InsertXmlItem: <%s>\n"), strName);

		// hack to stop display
		if (/*!strName.IsEmpty() &&*/ strName.Compare(_T("...")) != 0)
		{
			strName.TrimLeft();
			strName.TrimRight();

#if 0  // -----------------------------------------------------------
			if (strName.GetLength() <= 0)
			{
				TRACE(_T("ERROR - name is empty\n"));
				ASSERT(FALSE);
			}
#endif // -----------------------------------------------------------

			CString strIsCategory = pElement->GetValue(_T("category"));

			if (!strIsCategory.IsEmpty())
			{
				Task category;
				category.strCategory = strName;

				category.strName = CString("");
				category.oleDate = COleDateTime();
				category.iState = 0;

				pList->push_back(category);
			}
			else
			{
				Task item;
				item.strCategory = m_strCurrentCategory;

				CString strName = pElement->GetValue(_T("name"));
				if (!strName.IsEmpty())
					item.strName = strName;

				CString strDate = pElement->GetValue(_T("date"));
				if (!strDate.IsEmpty())
					item.oleDate = COleDateTime(atoi(strDate.Mid(6)), atoi(strDate.Mid(3, 2)), atoi(strDate.Mid(0, 2)), 1, 1, 1);
				else
					item.oleDate = COleDateTime();

				CString strChecked = pElement->GetValue(_T("checked"));
				if (!strChecked.IsEmpty())
					item.iState = _ttoi(strChecked);

				/*CString strEnabled = pElement->GetValue(_T("enabled"));
				if (!strEnabled.IsEmpty() && !item.bSeparator)
				item.bEnabled = _ttoi(strEnabled);*/

				TRACE(_T("category=<%s>, name=<%s>, <%s> \n"), m_strCurrentCategory, strName, strChecked);
				pList->push_back(item);
			}

			result = S_OK;
		}
	}

	return result;
}