/* JWinSearch

   Copyright(c) 2007, Jorge Peixoto Vasquez
*/

/* JWinSearch

   Copyright(c) 2007, Jorge Peixoto Vasquez
*/

#include "stdafx.h"
#include "resource_j.h"
#include "../fftabenumeratordefs.h"

#include "jwinsearch.h"

static HWND jws_hw_hwnd = 0;
static HWND jws_hwnd = 0;
static HWND jws_previous_fg_win;
static const CString jws_hw_classname = CString("JWinSearchHiddenClass");
static const CString jws_hw_winname = CString("JWinSearchHiddenWindow");
static wchar_t jffe_hid_win_classname[] = JFFE_HID_WIN_CLASSNAME;
static HINSTANCE jws_hinstance;
static std::vector<JWS_WINDOW_DATA> jws_windows;

static IShellWindowsPtr jws_shellwindows(0);

// Context when asking IE to enumerate its tabs
static HICON jws_ie_icon;
static HWND  jws_ie_hwnd;
static CString jws_ie_parent_name;

// Context when asking firefox to enumerate its tabs
static HICON jws_ff_icon;
static HWND  jws_ff_hwnd;
static CString jws_ff_parent_name;

// Context when asking chrome to enumerate its tabs
static HICON jws_chrome_icon;
static HWND  jws_chrome_hwnd;
static CString jws_chrome_parent_name;

//static std::vector<CRect>	chrome_rect_sort;
//static std::vector<CString> chrome_string_sort;

// Util
static std::string utf16ToUTF8(const std::wstring &s)
{
	const int size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, 0, NULL);

	std::vector<char> buf(size);
	::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &buf[0], size, 0, NULL);

	return std::string(&buf[0]);
}

static void removeAccent(TCHAR* s)
{
	int i = 0;
	while(s[i] != '\0') 
	{
		switch (s[i]) {
			case L'à':
			case L'ã':
			case L'á':
			case L'â':
				s[i] = 'a'; break;
			case L'À':
			case L'Ã':
			case L'Á':
			case L'Â':
				s[i] = 'A'; break;
			
			case L'è': 
			case L'é':
			case L'ê':
				s[i] = 'e'; break;
			case L'È':
			case L'É':
			case L'Ê':
				s[i] = 'E'; break;
							
			case L'ì':
			case L'í':
			case L'î':
				s[i] = 'i'; break;
			case L'Ì':
			case L'Í':
			case L'Î':
				s[i] = 'I'; break;

			case L'ò':
			case L'ô':
			case L'õ':
			case L'ó':
				s[i] = 'o'; break;
			case L'Ò':
			case L'Ô':
			case L'Õ':
			case L'Ó':
				s[i] = 'O'; break;
			
			case L'ù':
			case L'ú':
			case L'û':
			case L'ü':
				s[i] = 'u'; break;
			case L'Ù':
			case L'Ú':
			case L'Û':
			case L'Ü':
				s[i] = 'U'; break;
			
			case L'ç':
				s[i] = 'c'; break;
			case L'Ç':
				s[i] = 'C'; break;

			case L'•':
			case L'·':
			case L'–':
				s[i] = '-'; break;

			case L'’':
				s[i] = '\''; break;
		}
		
		i++;
	}
}

static void removeAccent(std::string* s)
{
	TCHAR *param = new TCHAR[s->size() + 1];
	param[s->size()] = 0;
	//As much as we'd love to, we can't use memcpy() because
	//sizeof(TCHAR)==sizeof(char) may not be true:
	try 
	{
		std::copy(s->begin(), s->end(), param);

		removeAccent(param);
		*s = std::string(param);
	}
	catch (int e)
	{
		TRACE("Exception using removeAccent: [%d]\n", e);
	}
}

// Used with IAccessible
std::string GetName(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
{
	if (!pAcc)
		return "";
	CComBSTR bstrName;
	HRESULT hr = pAcc->get_accName(varChild, &bstrName);
	if (FAILED(hr))
		return "";
	if (!bstrName.m_str)
		return "<NULL>";

	std::string str;
	int res = WideCharToMultiByte(CP_ACP, 0, bstrName.m_str, -1, NULL, 0, NULL, NULL);
	if (res > 0)
	{
		str.resize(res);
		WideCharToMultiByte(CP_ACP, 0, bstrName.m_str, -1, &str[0], res, NULL, NULL);
	}

	return str;
}

// Used with IAccessible
std::string GetRole(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
{
	if (!pAcc)
		return "";
	CComVariant varRoleID;
	HRESULT hr = pAcc->get_accRole(varChild, &varRoleID);
	if (FAILED(hr))
		return "";
	TCHAR sRoleBuff[1024] = { 0 };
	hr = ::GetRoleText(varRoleID.lVal, sRoleBuff, 1024);
	if (FAILED(hr))
		return "";
	return sRoleBuff;
}

// Used with IAccessible
HRESULT GetTabDescendants(HWND hWnd, JWS_WINDOW_TYPE type, CStringArray& inout_array)
{
	CStringArray strArray;
	HRESULT hr;
	IUIAutomation *_pUIAutomation;

	hr = CoCreateInstance(__uuidof(CUIAutomation),
			NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IUIAutomation),
			(void**)&_pUIAutomation);

	IUIAutomationElement *pElementBrowser = NULL;
	hr = _pUIAutomation->ElementFromHandle((UIA_HWND)hWnd, &pElementBrowser);

	IUIAutomationCondition *pConditionControlView;
	hr = _pUIAutomation->get_ControlViewCondition(&pConditionControlView);

	if (SUCCEEDED(hr))
	{
		// Now create a property condition specifying that we're interested in elements that
		// have a Control Type property of "tabItemControl".
		VARIANT varProp;
		varProp.vt = VT_I4;
		varProp.lVal = UIA_TabItemControlTypeId;

		IUIAutomationCondition *pConditionTabItem;
		hr = _pUIAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId, varProp, &pConditionTabItem);

		if (SUCCEEDED(hr))
		{
			// Now combine these two properties such that the search results only contain
			// elements that are in the Control view AND are hyperlinks. We would get the
			// same results here if we didn't include the Control view clause, (as tabItemControl
			// won't exist only in the Raw view), but by specifying that we're only interested
			// in the Control view, UIA won't bother checking all the elements that only exist
			// in the Raw view to see if they're tabItemControl.
			IUIAutomationCondition *pCondition = NULL;
			hr = _pUIAutomation->CreateAndCondition(pConditionControlView, pConditionTabItem, &pCondition);

			IUIAutomationElementArray *_pElementArray = NULL;
			hr = pElementBrowser->FindAll(TreeScope_Descendants, pCondition, &_pElementArray);
			
			if (SUCCEEDED(hr))
			{
				// Find the number of tabItemControl returned by the search. (The number of tabItemControl 
				// found might be zero if the browser window is minimized.)
				int cElements = 0;
				hr = _pElementArray->get_Length(&cElements);
				if (SUCCEEDED(hr) && (cElements > 0))
				{
					CString strParentName;
					
					switch (type)
					{
						case JWS_WINDOW_TYPE::JWS_WT_IE:
							strParentName = jws_ie_parent_name;
							break;
						case JWS_WINDOW_TYPE::JWS_WT_FIREFOX:
							strParentName = jws_ff_parent_name;
							break;
						case JWS_WINDOW_TYPE::JWS_WT_CHROME:
							strParentName = jws_chrome_parent_name;
							break;
						default:
							return S_FALSE;
					}

					// Process each returned tabItemControl element.
					for (int idxElement = 0; idxElement < cElements; idxElement++)
					{
						IUIAutomationElement *pElement = NULL;
						hr = _pElementArray->GetElement(idxElement, &pElement);
						if (FAILED(hr))
							break;
						
						// Get the name property for the tabItemControl element. How we get this depends
						// on whether we requested that the property should be cached or not.
						BSTR bstrName = NULL;
						
						// Call back to the target app to retrieve the bounds of the tabItemControl element.
						RECT rcBounds;
						HRESULT hr = pElement->get_CurrentBoundingRectangle(&rcBounds);

						if (SUCCEEDED(hr))
						{
							// If we're the tabItemControl has a zero-size bounding rect, ignore the element.
							if ((rcBounds.right > rcBounds.left) && (rcBounds.bottom > rcBounds.top))
							{
								// Get the name of the element. This will normally be the text shown on the screen.
								BSTR bstrName;
								hr = pElement->get_CurrentName(&bstrName);
								
								// If we have non-null name, notify the main UI thread. (This sample does not check
								// for names that only contains whitespace.)
								if (SUCCEEDED(hr) && (bstrName != NULL))
								{
									CString strName = CString(bstrName);
									/*std::string str = utf16ToUTF8(bstrName);
									removeAccent(&str);
									strName.Format("%s", str.c_str());*/

									//TRACE("FOI! Olha o nome: %s\n", strName);

									if (strParentName.Find(strName) < 0)
									{
										//// Variables related to CRect
										//std::vector<CRect>::iterator it_rect, begin_rect, end_rect;
										//it_rect = begin_rect = chrome_rect_sort.begin();
										//end_rect = chrome_rect_sort.end();
										//int size = chrome_rect_sort.size();
										//// Variable related to String
										//std::vector<CString>::iterator begin_string = chrome_string_sort.begin();
										//bool inserted = false;
										//for (int i = 0; i < size; i++)
										//{
										//	if (rcBounds.left < it_rect->left)
										//	{
										//		chrome_rect_sort.insert(begin_rect + i, rcBounds);
										//		chrome_string_sort.insert(begin_string + i, strName);
										//		inserted = true;
										//		break;
										//	}
										//	std::advance(it_rect, 1);
										//}
										//if (!inserted)
										//{
										//	chrome_rect_sort.push_back(rcBounds);
										//	chrome_string_sort.push_back(strName);
										//}

										inout_array.Add(strName);
									}
								}
							}
						}
					}
				}
			}

			if (pCondition != NULL)
			{
				pCondition->Release();
				pCondition = NULL;
			}

			pConditionTabItem->Release();
			pConditionTabItem = NULL;
		}

		pConditionControlView->Release();
		pConditionControlView = NULL;
	}
	
	_pUIAutomation->Release();
	_pUIAutomation = NULL;

	return hr;
}

static void jws_enum_ff_tabs(const CString& parent_name, HWND hwnd, HICON hicon)
{
	CComPtr<IAccessible> pAccFF = NULL;
	HRESULT hr = ::AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IAccessible, (void**)(&pAccFF));

	CString dummy;
	if ((hr == S_OK) && (pAccFF != NULL))
	{
		// Process all message for up to JWS_FF_TIMEOUT or an answer is received
		jws_ff_icon = hicon;
		jws_ff_hwnd = hwnd;
		jws_ff_parent_name = parent_name;

		// Walk through MSAA tree to search for all tabs opened
		CStringArray strArray;
		if (SUCCEEDED(GetTabDescendants(hwnd, JWS_WINDOW_TYPE::JWS_WT_FIREFOX, strArray)))
		{
			// Add to list
			int size = strArray.GetCount();
			CString dummy;
			for (int i = 0; i < size; i++)
			{
				dummy = strArray.GetAt(i);
				TRACE("* %d - %s (Object)\n", i, dummy);

				jws_windows.push_back(JWS_WINDOW_DATA(jws_ff_parent_name, jws_ff_hwnd, jws_ff_icon,
					dummy, i, JWS_WINDOW_TYPE::JWS_WT_FIREFOX));
			}
		}
	}
}

static void jws_enum_ie_tabs(const CString &parent_name, HWND hwnd, HICON hicon)
{
	CComPtr<IAccessible> pAccIE = NULL;
	HRESULT hr = ::AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IAccessible, (void**)(&pAccIE));

	CString dummy;
	if ((hr == S_OK) && (pAccIE != NULL))
	{
		// Process all message for up to JWS_FF_TIMEOUT or an answer is received
		jws_ie_icon = hicon;
		jws_ie_hwnd = hwnd;
		jws_ie_parent_name = parent_name;

		// Walk through MSAA tree to search for all tabs opened
		CStringArray strArray;
		if (SUCCEEDED(GetTabDescendants(hwnd, JWS_WINDOW_TYPE::JWS_WT_IE, strArray)))
		{
			// Add to list
			int size = strArray.GetCount();
			CString dummy;
			for (int i = 0; i < size; i++)
			{
				dummy = strArray.GetAt(i);
				TRACE("* %d - %s (Object)\n", i, dummy);

				jws_windows.push_back(JWS_WINDOW_DATA(jws_ie_parent_name, jws_ie_hwnd, jws_ie_icon,
					dummy, i, JWS_WINDOW_TYPE::JWS_WT_IE));
			}
		}
	}
}

static void jws_enum_chrome_tabs(const CString& parent_name, HWND hwnd, HICON hicon)
{
	CComPtr<IAccessible> pAccChrome = NULL;
	HRESULT hr = ::AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IAccessible, (void**)(&pAccChrome));

	if ((hr == S_OK) && (pAccChrome != NULL))
	{
		// fill variables
		jws_chrome_icon = hicon;
		jws_chrome_hwnd = hwnd;
		jws_chrome_parent_name = parent_name;

		// clear all structures
		//chrome_rect_sort.clear();
		//chrome_string_sort.clear();

		// Walk through MSAA tree to search for all tabs opened
		CStringArray strArray;
		if (SUCCEEDED(GetTabDescendants(hwnd, JWS_WINDOW_TYPE::JWS_WT_CHROME, strArray)))
		{
			// Add to list
			int size = strArray.GetCount();
			CString dummy;
			for (int i = 0; i < size; i++)
			{
				dummy = strArray.GetAt(i);
				TRACE("* %d - %s (Object)\n", i, dummy);

				jws_windows.push_back(JWS_WINDOW_DATA(jws_chrome_parent_name, jws_chrome_hwnd, jws_chrome_icon,
					dummy, i, JWS_WINDOW_TYPE::JWS_WT_CHROME));
			}
		}
	}
}

//jws_enum_notepad_tabs

static void select_tab(JWS_WINDOW_DATA *w)
{
	HRESULT hr;
	IUIAutomation *_pUIAutomation;

	hr = CoCreateInstance(__uuidof(CUIAutomation),
		NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IUIAutomation),
		(void**)&_pUIAutomation);

	IUIAutomationElement *pElementBrowser = NULL;
	hr = _pUIAutomation->ElementFromHandle((UIA_HWND)w->hwnd, &pElementBrowser);

	IUIAutomationElement* pTab;
	IUIAutomationCondition* pConditionStringName = NULL;
	IUIAutomationCondition* pConditionControlType = NULL;
	IUIAutomationCondition* pConditionCombined = NULL;

	VARIANT varProp;

	// First condition
	varProp.vt = VT_BSTR;
	varProp.bstrVal = w->name.AllocSysString();
	hr = _pUIAutomation->CreatePropertyCondition(UIA_NamePropertyId, varProp, &pConditionStringName);
	::SysFreeString(varProp.bstrVal);
	
	// Second Condition
	varProp.vt = VT_I4;
	varProp.lVal = UIA_TabItemControlTypeId;
	hr = _pUIAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId, varProp, &pConditionControlType);

	// Combine both conditions
	hr = _pUIAutomation->CreateAndCondition(pConditionStringName, pConditionControlType, &pConditionCombined);

	// Find item according to conditions
	hr = pElementBrowser->FindFirst(TreeScope_Descendants, pConditionCombined, &pTab);

	if (SUCCEEDED(hr) && (pTab != NULL))
	{
		// Get its LegacyIAccessible in order to call DoDefaultAction and active this tab
		IUIAutomationLegacyIAccessiblePattern* pSelectTab;
		hr = pTab->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId,
			__uuidof(IUIAutomationLegacyIAccessiblePattern),
			(void **)&pSelectTab);

		if ((SUCCEEDED(hr)) && (pSelectTab != NULL))
		{
			DWORD currentState;
			hr = pSelectTab->get_CurrentState(&currentState);

			if(SUCCEEDED(hr))
				if(currentState != STATE_SYSTEM_SELECTED)
					hr = pSelectTab->DoDefaultAction();
	
			pSelectTab->Release();
		}

		pTab->Release();
	}

	pConditionCombined->Release();
	pConditionStringName->Release();
	pConditionControlType->Release();
	
	_pUIAutomation->Release();
}

// Horrible way of selecting Chrome tab. Couldn't find anything better.
/*static void select_tab(JWS_WINDOW_DATA *w, JWS_WINDOW_TYPE type)
{
	int tab_index = 0;
	switch (type)
	{
		case JWS_WINDOW_TYPE::JWS_WT_IE:
			tab_index = w->ie.tab_index;
			break;
		case JWS_WINDOW_TYPE::JWS_WT_FIREFOX:
			tab_index = w->firefox.tab_index;
			break;
		case JWS_WINDOW_TYPE::JWS_WT_CHROME:
			tab_index = w->chrome.tab_index;
			break;
		default:
			return;
	}

	int n_tab_tries, n_tries = JWS_MAX_IE_RAISE_TIME;
	bool ctrl_tab_enabled = false;

	// Wait until IE Window raises
	while (GetForegroundWindow() != w->hwnd && n_tries-- > 0)
		Sleep(1);
	if (n_tries <= 0) // give up
		return;

	// Send CTRL+INDEX until the correct tab is active
	INPUT ctrl_index[4];

	ctrl_index[0].type = INPUT_KEYBOARD;
	ctrl_index[0].ki.wVk = VK_CONTROL;
	ctrl_index[0].ki.wScan = 0x1d;
	ctrl_index[0].ki.dwFlags = 0;
	ctrl_index[0].ki.dwExtraInfo = 0;
	ctrl_index[0].ki.time = 0;

	ctrl_index[1] = ctrl_index[0];
	if (tab_index < 9)
		ctrl_index[1].ki.wVk = '0' + tab_index;
	else
	{
		ctrl_index[1].ki.wVk = '0' + 8;
		ctrl_tab_enabled = true;
	}

	ctrl_index[1].ki.wScan = 0x0f;
	
	ctrl_index[2] = ctrl_index[1];
	ctrl_index[2].ki.dwFlags |= KEYEVENTF_KEYUP;

	ctrl_index[3] = ctrl_index[0];
	ctrl_index[3].ki.dwFlags |= KEYEVENTF_KEYUP;

	//send it
	UINT n_keys_sent;
	for (n_keys_sent = 0; n_keys_sent < 4; n_keys_sent += SendInput(4 - n_keys_sent, ctrl_index + n_keys_sent, sizeof(INPUT)), Sleep(1));

	// even if we send it, we will need to use CTRL + TAB
	if (ctrl_tab_enabled)
	{
		// Send CTRL+INDEX until the correct tab is active
		INPUT ctrl_tab[4];

		ctrl_tab[0].type = INPUT_KEYBOARD;
		ctrl_tab[0].ki.wVk = VK_CONTROL;
		ctrl_tab[0].ki.wScan = 0x1d;
		ctrl_tab[0].ki.dwFlags = 0;
		ctrl_tab[0].ki.dwExtraInfo = 0;
		ctrl_tab[0].ki.time = 0;

		ctrl_tab[1] = ctrl_tab[0];
		ctrl_tab[1].ki.wVk = VK_TAB;
		ctrl_tab[1].ki.wScan = 0x0f;

		ctrl_tab[2] = ctrl_tab[1];
		ctrl_tab[2].ki.dwFlags |= KEYEVENTF_KEYUP;

		ctrl_tab[3] = ctrl_tab[0];
		ctrl_tab[3].ki.dwFlags |= KEYEVENTF_KEYUP;

		n_tab_tries = tab_index - 8;

		HWND cur_tab_hwnd = FindWindowEx(w->hwnd, 0, "TabWindowClass", 0);
		HWND new_tab_hwnd;

		Sleep(JWS_MIN_IE_TAB_SWITCH_TIME);

		while (n_tab_tries-- > 0)
		{
			TRACE("tab_index = %d n_tab_tries = %d\n", tab_index, n_tab_tries);

			// If IE goes ou of focus, don't send CTRL+TAB to anyone
			if (GetForegroundWindow() != w->hwnd)
				return;

			// Send the 4 keys
			for (n_keys_sent = 0; n_keys_sent < 4; n_keys_sent += SendInput(4 - n_keys_sent, ctrl_tab + n_keys_sent, sizeof(INPUT)), Sleep(1));

			// Give IE a chance
			Sleep(JWS_MIN_IE_TAB_SWITCH_TIME);

			// Wait for the TAB to change
			for (n_tries = JWS_MAX_IE_TAB_SWITCH_TIME / 5; cur_tab_hwnd == (new_tab_hwnd = FindWindowEx(w->hwnd, 0, "TabWindowClass", 0)) && n_tries > 0; n_tries--) Sleep(5);
			if (n_tries > 0) // OK, tab switched
				cur_tab_hwnd = new_tab_hwnd;
		}
	}
}*/

static BOOL CALLBACK jws_enum_windows_cb(HWND hwnd, LPARAM )
{
	wchar_t title[512];
	TCHAR titleDummy[512];

	DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	if(!(dwStyle & WS_VISIBLE))
	return TRUE;
  
	GetWindowText(hwnd, titleDummy, _countof(titleDummy));
  
	if(!titleDummy[0])
		return TRUE;
  
	removeAccent(titleDummy);

	size_t outSize;
	mbstowcs_s(&outSize, title, titleDummy, 512);

	LONG exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
  
	if(exstyle & WS_EX_TOOLWINDOW)
		return TRUE;
  
	HICON hicon = (HICON)SendMessage(hwnd, WM_GETICON, JWS_WND_ICON_TYPE, 0);
  
	if(!hicon)
		hicon = (HICON)(UINT_PTR)GetClassLongPtr(hwnd, JWS_CLS_ICON_TYPE);
  
	if(!hicon)
		hicon = LoadIcon(NULL, IDI_APPLICATION);
  
	jws_windows.push_back(JWS_WINDOW_DATA(hwnd, hicon, title));

	wchar_t className[512];
	TCHAR classNameDummy[512];
	if(GetClassName(hwnd, classNameDummy, _countof(classNameDummy)))
	{
		mbstowcs_s(&outSize, className, classNameDummy, 512);
	
		if(!wcscmp(className, L"MozillaUIWindowClass"))
			jws_enum_ff_tabs(CString(title), hwnd, hicon);
		else if(!(wcscmp(className, L"IEFrame")))
			jws_enum_ie_tabs(CString(title), hwnd, hicon);
		else if (!(wcscmp(className, L"Chrome_WidgetWin_1")))
			jws_enum_chrome_tabs(CString(title), hwnd, hicon);
		/*else if (!(wcscmp(className, L"Notepad++")))
			jws_enum_notepad_tabs(CString(title), hwnd, hicon);*/
	}

	return TRUE;
}

static void jws_display_window_list()
{
	HWND lv_hwnd = GetDlgItem(jws_hwnd, IDC_LIST);
	const JWS_WINDOW_DATA *base, *w, *w_end;
  
	wchar_t filter[128];
	TCHAR filterDummy[128];
	GetDlgItemText(jws_hwnd, IDC_EDIT, filterDummy, _countof(filterDummy));
  
	size_t outSize;
	mbstowcs_s(&outSize, filter, filterDummy, 128);
      
	ListView_DeleteAllItems(lv_hwnd);

	LRESULT ret;

	int n_item = 0;
	for(w = base = &jws_windows.front(), w_end = w + jws_windows.size(); w < w_end; w++)
	{
		CStringW csw(w->name);
		if(filter[0] && !stristrW(csw, filter))
			continue;

		LVITEM it;
		memset(&it, 0, sizeof(LVITEM));
		it.iItem = n_item++;
		it.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM | LVIF_INDENT;
		it.iSubItem = 0;

		//std::string utf8String = utf16ToUTF8(w->name.c_str());
		//it.pszText = (LPSTR)utf8String.c_str();
		CString dummy(w->name);
		it.pszText = dummy.GetBuffer();
		dummy.ReleaseBuffer();
		
		it.cchTextMax = (int)dummy.GetLength() + 1;
		it.iImage = w->icon_index;
		it.lParam = (LPARAM)w; 
		it.iIndent = w->type == JWS_WT_NORMAL ? 0 : 1;

		ret = ListView_InsertItem(lv_hwnd, &it);
	}
	ListView_SetColumnWidth(lv_hwnd, 0, LVSCW_AUTOSIZE);
}

static INT_PTR on_task_selected(int item_idx)
{
	JWS_WINDOW_DATA *w;
	HWND fg_hwnd;
	LVITEM item;

	ShowWindow(jws_hwnd, SW_HIDE);
	memset(&item, 0, sizeof(LVITEM));
	item.iItem = item_idx;
	item.mask = LVIF_PARAM ;
	ListView_GetItem(GetDlgItem(jws_hwnd, IDC_LIST), &item);

	w = (JWS_WINDOW_DATA *)item.lParam;
	BringWindowToTop(w->hwnd);
	SetWindowPos(w->hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	fg_hwnd = GetForegroundWindow();
	
	if(!SetForegroundWindow(w->hwnd))
	{
		if(!fg_hwnd)
			fg_hwnd = FindWindow("Shell_TrayWnd", NULL);

		DWORD pid_fg_win = GetWindowThreadProcessId(fg_hwnd, NULL);
		DWORD pid_win = GetWindowThreadProcessId(w->hwnd, NULL);
		{
			wchar_t title[256];
			GetWindowText(w->hwnd, (LPSTR)title, _countof(title));
			title[255] = 0;
		}

		AttachThreadInput(pid_fg_win, pid_win, TRUE);
		SetForegroundWindow(w->hwnd);
		AttachThreadInput(pid_fg_win, pid_win, FALSE);
	}

	if(IsIconic(w->hwnd))
		SendMessage(w->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);

	ShowWindow(jws_hwnd, SW_HIDE);

	bool mustSelectTabToActive = false;
	if (w->type == JWS_WT_FIREFOX)
	{
		TRACE("Firefox tab selected tab_index: %d title: %s\n", w->chrome.tab_index, w->name);
		mustSelectTabToActive = true;
	}
	else if (w->type == JWS_WT_IE)
	{
		TRACE("IE tab selected tab_index: %d title: %s\n", w->ie.tab_index, w->name);
		mustSelectTabToActive = true;
	}
	else if (w->type == JWS_WT_CHROME)
	{
		TRACE("Chrome tab selected tab_index: %d title: %s\n", w->chrome.tab_index, w->name);
		mustSelectTabToActive = true;
	}

	if(mustSelectTabToActive)
		select_tab(w);

	EndDialog(jws_hwnd, 0);
	return TRUE;
}

static LRESULT CALLBACK jws_edit_subproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR , DWORD_PTR)
{
	switch(msg)
	{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
		case WM_KEYDOWN:
		if(wparam == VK_ESCAPE)
		{
			EndDialog(jws_hwnd, 0);
			SetForegroundWindow(jws_previous_fg_win);
			return 0;
		}
		else if(wparam == VK_DOWN || wparam == VK_TAB)
		{
			HWND lv_hwnd = GetDlgItem(jws_hwnd, IDC_LIST);
			if(!ListView_GetItemCount(lv_hwnd))
				return 0;
			ListView_SetItemState(lv_hwnd, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			SetFocus(lv_hwnd);
			return 0;
		}
		else if(wparam == VK_UP)
		{
			HWND lv_hwnd = GetDlgItem(jws_hwnd, IDC_LIST);
			int count;
			if(!(count = ListView_GetItemCount(lv_hwnd)))
				return 0;
			ListView_SetItemState(lv_hwnd, count - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			SetFocus(lv_hwnd);
			return 0;
		}
		else if(wparam == VK_RETURN)
		{
			HWND lv_hwnd = GetDlgItem(jws_hwnd, IDC_LIST);
			if(!ListView_GetItemCount(lv_hwnd))
			{
				EndDialog(jws_hwnd, 0);
				SetForegroundWindow(jws_previous_fg_win);
				return 0;
			}
			on_task_selected(0);
			return 0;
		}
		break;
	}

	return DefSubclassProc(hwnd, msg, wparam, lparam);
}

static LRESULT CALLBACK jws_list_subproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR , DWORD_PTR)
{
	switch(msg)
	{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
		case WM_KEYDOWN:
			if(wparam == VK_ESCAPE)
			{
				EndDialog(jws_hwnd, 0);
				SetForegroundWindow(jws_previous_fg_win);
				return 0;
			}
	}

	return DefSubclassProc(hwnd, msg, wparam, lparam);
}

static INT_PTR on_init_dlg(HWND hwnd)
{
	jws_hwnd = hwnd;
	if(!SetWindowSubclass(GetDlgItem(hwnd, IDC_EDIT), jws_edit_subproc, 0, 0))
		EndDialog(hwnd, 1);
	if(!SetWindowSubclass(GetDlgItem(hwnd, IDC_LIST), jws_list_subproc, 0, 0))
		EndDialog(hwnd, 1);

	EnumWindows(&jws_enum_windows_cb, 0);
	std::sort(jws_windows.begin(), jws_windows.end());

	LVCOLUMN col;
	memset(&col, 0, sizeof(LVCOLUMN));
	ListView_InsertColumn(GetDlgItem(jws_hwnd, IDC_LIST), 0, &col);

	HIMAGELIST image_list = ImageList_Create(JWS_ICON_SIZE, JWS_ICON_SIZE, ILC_MASK | ILC_COLOR32, (int)jws_windows.size(), 0);
	JWS_WINDOW_DATA *w, *w_end;
	int index = 0;
	for(w = &jws_windows.front(), w_end = w + jws_windows.size(); w < w_end; w++)
		w->icon_index = ImageList_AddIcon(image_list, w->icon);
	ListView_SetImageList(GetDlgItem(jws_hwnd, IDC_LIST), image_list, JWS_IMAGE_LIST_SIZE);
	jws_display_window_list();
	SetForegroundWindow(hwnd);
	SetFocus(GetDlgItem(hwnd, IDC_EDIT));
	SetActiveWindow(hwnd);

	return TRUE;
}

static INT_PTR CALLBACK jws_dlg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	NMITEMACTIVATE *ia;

	switch(msg)
	{
		case WM_DESTROY:
			RemoveWindowSubclass(GetDlgItem(hwnd, IDC_EDIT), jws_edit_subproc, 0);
			RemoveWindowSubclass(GetDlgItem(hwnd, IDC_LIST), jws_list_subproc, 0);
			jws_hwnd = 0;
			jws_windows.clear();
			jws_shellwindows = 0;
			
			return TRUE;
		case WM_INITDIALOG:
			return on_init_dlg(hwnd);
		case WM_ACTIVATE:
			if(wparam != WA_INACTIVE)
				break;
			EndDialog(hwnd, 0);
			return TRUE;
		case WM_NOTIFY:
			if((int)wparam != IDC_LIST)
				break;
			ia = (NMITEMACTIVATE *)lparam;
			if(ia->hdr.code != LVN_ITEMACTIVATE)
				break;
			return on_task_selected(ia->iItem);
		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
			case IDC_EDIT:
				if(HIWORD(wparam) != EN_UPDATE)
				break;
				jws_display_window_list();
				return TRUE;
			case IDC_BTN_QUIT:
				/*if(IDYES == MessageBox(jws_hwnd, "Stop running JWinSearch until next reboot?", "Quit?", MB_YESNO | MB_ICONQUESTION))
				PostQuitMessage(0);*/
				EndDialog(jws_hwnd, 0);
				SetForegroundWindow(jws_previous_fg_win);
				return TRUE;
			default:
				break;
			}
			break;
		case WM_COPYDATA:
			//return on_ff_answer((HWND)wparam, (COPYDATASTRUCT *)lparam);
			break;
	}

	return FALSE;
}

static LRESULT CALLBACK jws_hw_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_HOTKEY:
			if(wparam != IDH_ALT_WIN)
				return FALSE;
			if(jws_hwnd)
			{
				EndDialog(jws_hwnd, 0);
				SetForegroundWindow(jws_previous_fg_win);
				return TRUE;
			}
			jws_previous_fg_win = GetForegroundWindow();
			DialogBox(jws_hinstance, MAKEINTRESOURCE(IDD_JWS_DLG), jws_hw_hwnd, &jws_dlg_proc);
			return TRUE;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

/* ################# CJWinSearch Thread #################### */
IMPLEMENT_DYNCREATE(CJWinSearch, CWinThread)

CJWinSearch::CJWinSearch()
{

}

CJWinSearch::~CJWinSearch()
{
}

BOOL CJWinSearch::InitInstance()
{
	return TRUE;
}

int CJWinSearch::ExitInstance()
{

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CJWinSearch, CWinThread)
	//{{AFX_MSG_MAP(CAppBarMngr)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

UINT CJWinSearch::Init()
{
	HINSTANCE hInstance = AfxGetInstanceHandle();
	LPWSTR lpCmdLine = L"";

	if ((jws_hw_hwnd = FindWindowEx(HWND_MESSAGE, 0, jws_hw_classname, jws_hw_winname)))
	{
		if (!wcscmp(lpCmdLine, L"/STOP"))
			PostMessage(jws_hw_hwnd, WM_QUIT, 0, 0);
		else
			PostMessage(jws_hw_hwnd, WM_HOTKEY, IDH_ALT_WIN, MOD_WIN);
		return 0;
	}
	if (!wcscmp(lpCmdLine, L"/STOP"))
		return 0;
	if (!wcscmp(lpCmdLine, L"/INSTALL"))
	{
		wchar_t cmd_line[128];
		wcscpy_s(cmd_line, L"JWinSearch.exe");
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		CreateProcess(0, (LPSTR)cmd_line, 0, 0, 0, 0, 0, 0, &si, &pi);
		return 0;
	}

	jws_hinstance = hInstance;

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icc);

	CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	WNDCLASSEX wc;

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = jws_hw_proc;
	wc.hInstance = hInstance;
	wc.lpszClassName = jws_hw_classname;

	if (!RegisterClassEx(&wc))
		return 10;

	::CoInitialize(0);
	
	// Centralize the window
	RECT rc;	
	int width = 200;
	int height = 200;

	GetClientRect(GetDesktopWindow(), &rc);
	rc.left = (rc.right / 2) - (width / 2);
	rc.top = (rc.bottom / 2) - (height / 2);

	CStringW csw_classname(jws_hw_classname), csw_winname(jws_hw_winname);
	jws_hw_hwnd = CreateWindowExW(0, csw_classname, csw_winname, WS_OVERLAPPED, rc.left, rc.top, width, height, HWND_MESSAGE, 0, hInstance, 0);
	if (!jws_hw_hwnd)
	{
		UnregisterClass(jws_hw_classname, hInstance);
		return 15;
	}

	// Creates Dialog
	ShowWindow(jws_hw_hwnd, SW_SHOW);

	// Register hotkey
	if (!RegisterHotKey(jws_hw_hwnd, IDH_ALT_WIN, MOD_WIN, LOBYTE(VkKeyScan(L'a'))))
	{
		DWORD err = GetLastError();
		DestroyWindow(jws_hw_hwnd);
		return 30;
	}

	MSG msg;
	BOOL ret;

	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (ret == -1)
			return 9999;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	::CoUninitialize();

	return 0;
}