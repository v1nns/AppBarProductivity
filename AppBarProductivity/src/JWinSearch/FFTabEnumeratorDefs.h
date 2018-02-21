#pragma once

#define JFFE_HID_WIN_CLASSNAME       L"JFFEHiddenWinClass"

// wparam = win handle to receive WM_COPYDATA, lparam = HWND of the enumerated window
#define JFFE_WM_ENUMERATE       (WM_USER + 40) 
// wparam = wparam = win handle to receive WM_COPYDATA, win handle of plugin hidden window
#define JFFE_WM_REPLY_ENUMERATE (WM_USER + 41) 
// wparam = (int)tab_index, lparam = HWND of the enumerated window
#define JFFE_WM_ACTIVATE        (WM_USER + 42)

#define JFFE_TAB_NAME_LEN 128
#define JFFE_TAB_URI_LEN  128

struct JFFE_TabInfo
{
  wchar_t tab_name[JFFE_TAB_NAME_LEN];
  wchar_t tab_uri[JFFE_TAB_URI_LEN];
  int tab_id;
};