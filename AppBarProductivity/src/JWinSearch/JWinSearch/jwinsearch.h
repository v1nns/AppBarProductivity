// Just make using COM easier
//_COM_SMARTPTR_TYPEDEF(IShellWindows, IID_IShellWindows);
//_COM_SMARTPTR_TYPEDEF(IWebBrowser2, IID_IWebBrowser2);

#include "../FFTabEnumeratorDefs.h"
#include <afxwin.h>
#include <map>

// All timeout in milliseconds
#define JWS_MIN_IE_TAB_SWITCH_TIME 30   // Minimum time IE takes to change the TAB
#define JWS_MAX_IE_TAB_SWITCH_TIME 100  // Maximum time to wait for IE to process CTRL + TAB before resending CTRL+TAB
#define JWS_MAX_IE_RAISE_TIME      500  // Maximum time to wait for IE window to appear 
#define JWS_FF_TIMEOUT            1100  // Maximum time to wait for firefox to answer 

#ifdef JWS_BIG_ICONS
#  define JWS_ICON_SIZE 32
#  define JWS_WND_ICON_TYPE ICON_BIG
#  define JWS_CLS_ICON_TYPE GCLP_HICON
#  define JWS_IMAGE_LIST_SIZE LVSIL_NORMAL
#else
#  define JWS_ICON_SIZE 16
#  define JWS_WND_ICON_TYPE ICON_SMALL 
#  define JWS_CLS_ICON_TYPE GCLP_HICONSM
#  define JWS_IMAGE_LIST_SIZE LVSIL_SMALL
#endif

#define IDH_ALT_WIN 55

enum JWS_WINDOW_TYPE
{
	JWS_WT_NORMAL,   // SO Window
	JWS_WT_CHROME,	 // Chrome TAB	
	JWS_WT_FIREFOX,  // Firefox TAB
	JWS_WT_IE       // Internet Explorer TAB
};

struct JWS_WINDOW_DATA
{
	JWS_WINDOW_DATA(const JWS_WINDOW_DATA &other)
	{
		*this = other;
	}

	// Standard windows
	JWS_WINDOW_DATA(HWND _hwnd, HICON hicon, const wchar_t *_name)
	: hwnd(_hwnd), icon(hicon), name(_name), type(JWS_WT_NORMAL) { }

	// Window with tabs
	JWS_WINDOW_DATA(const CString &_parent_name, HWND main_hwnd, HICON hicon, const CString &_tab_title, int tab_index, JWS_WINDOW_TYPE tab_type)
		: hwnd(main_hwnd), icon(hicon), type(tab_type), parent_name(_parent_name)
	{
		name = _tab_title;

		switch (tab_type) 
		{
			case JWS_WINDOW_TYPE::JWS_WT_CHROME:
				chrome.tab_index = tab_index;
				break;
			case JWS_WINDOW_TYPE::JWS_WT_FIREFOX:
				firefox.tab_index = tab_index;
				break;
			case JWS_WINDOW_TYPE::JWS_WT_IE:
				ie.tab_index = tab_index;
				break;
			default:
				break;
		}
	}

	JWS_WINDOW_DATA::~JWS_WINDOW_DATA()
	{
		
	}

	void operator = (const JWS_WINDOW_DATA &other)
	{
		name = other.name;
		hwnd = other.hwnd;
		icon = other.icon;
		type = other.type;
		parent_name = other.parent_name;
		if(type == JWS_WT_IE)
		{
			ie.tab_index = other.ie.tab_index;
		}
		else if(type == JWS_WT_FIREFOX)
		{
			firefox.tab_index = other.firefox.tab_index;
		}
		else if (type == JWS_WT_CHROME)
		{
			chrome.tab_index = other.chrome.tab_index;
		}
	}

	int icon_index;  // inside the listview's image list

	HWND hwnd;
	HICON icon;
	CString name;
	CString parent_name;
	JWS_WINDOW_TYPE type;
	union 
	{
		struct
		{
			int tab_index;
		} firefox;
		struct
		{
			int tab_index;
		} chrome;
		struct
		{
			int tab_index;
		} ie;
	} ;

	CString comp_name(void) const
	{
		if(type == JWS_WT_NORMAL)
			return name;
		return parent_name + name;
	}
  
	bool operator  < (const JWS_WINDOW_DATA &d) const 
	{ 
		return comp_name() < d.comp_name(); 
	}
};

class CJWinSearch : public CWinThread
{
	DECLARE_DYNCREATE(CJWinSearch)
public:
	CJWinSearch(); // protected constructor used by dynamic creation
	// Attributes
	
	// Operations
public:
	UINT Init();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppBarMngr)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CJWinSearch();

	// Generated message map functions
	//{{AFX_MSG(CAppBarMngr)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
