#include <windows.h>
#include <commctrl.h>
#include "header.h"

TabBarClass TabBar;
const int TAB_FONT_HEIGTH = 16;
wchar_t TAB_FONT_FACE[] = L"Tahoma";
WNDPROC _defaultTabProc = NULL;

TabBarClass::TabBarClass()
{
	_active = false;
	_hwndTab = NULL;
	_tabHeight = NULL; memset(&m_Margins, 0, sizeof(m_Margins));
	_titleShouldChange = false;
	_prevTab = -1;
	mb_ChangeAllowed = FALSE;
	mb_Enabled = TRUE;
	mh_ToolbarParent = NULL; mh_Toolbar = NULL;
	//GetConsolesTitles = NULL;
	//ActivateConsole = NULL;
	ConMan_KeyAction = NULL;
}

void TabBarClass::Enable(BOOL abEnabled)
{
	if (_hwndTab && mb_Enabled!=abEnabled)
	{
		EnableWindow(_hwndTab, abEnabled);
		mb_Enabled = abEnabled;
	}
}

void TabBarClass::Refresh(BOOL abFarActive)
{
	Enable(abFarActive);
}

void TabBarClass::Reset()
{
	if (!_active)
	{
		return;
	}

	ConEmuTab tab; memset(&tab, 0, sizeof(tab));
	tab.Pos=0;
	tab.Current=1;
	tab.Type = 1;
	TabBar.Update(&tab, 1);
}

void TabBarClass::Retrieve()
{
	CConEmuPipe pipe;
	if (pipe.Init(_T("TabBarClass::Retrieve"), TRUE))
	{
		DWORD cbWritten=0;
		if (pipe.Execute(CMD_REQTABS))
		{
			gConEmu.DnDstep(_T("Tabs: Checking for plugin (1 sec)"));
			// �������� ��������, �������� ��� ������ �����
			cbWritten = WaitForSingleObject(pipe.hEventAlive, CONEMUALIVETIMEOUT);
			if (cbWritten!=WAIT_OBJECT_0) {
				TCHAR szErr[MAX_PATH];
				wsprintf(szErr, _T("ConEmu plugin is not active!\r\nProcessID=%i"), pipe.nPID);
				MBoxA(szErr);
			} else {
				gConEmu.DnDstep(_T("Tabs: Waiting for result (10 sec)"));
				cbWritten = WaitForSingleObject(pipe.hEventReady, CONEMUREADYTIMEOUT);
				if (cbWritten!=WAIT_OBJECT_0) {
					TCHAR szErr[MAX_PATH];
					wsprintf(szErr, _T("Command waiting time exceeds!\r\nConEmu plugin is locked?\r\nProcessID=%i"), pipe.nPID);
					MBoxA(szErr);
				} else {
					gConEmu.DnDstep(_T("Tabs: Recieving data"));
					DWORD cbBytesRead=0;
					int nTabCount=0;
					pipe.Read(&nTabCount, sizeof(nTabCount), &cbBytesRead);

					if (nTabCount<=0) {
						gConEmu.DnDstep(_T("Tabs: data empty"));
						this->Reset();
					} else {
						COPYDATASTRUCT cds = {0};
						
						cds.dwData = nTabCount;
						cds.lpData = pipe.GetPtr(); // �����

						gConEmu.OnCopyData(&cds);
						gConEmu.DnDstep(NULL);
					}
				}
			}
		}
	}
}

void TabBarClass::AddTab(wchar_t* text, int i)
{
	TCITEM tie;
	tie.mask = TCIF_TEXT;
	tie.iImage = -1; 
	tie.pszText = text ;

	int nCurCount = TabCtrl_GetItemCount(_hwndTab);
	if (i>=nCurCount)
		TabCtrl_InsertItem(_hwndTab, i, &tie);
	else
		TabCtrl_SetItem(_hwndTab, i, &tie);
}

void TabBarClass::SelectTab(int i)
{
    mb_ChangeAllowed = TRUE;
	TabCtrl_SetCurSel(_hwndTab, i);
	mb_ChangeAllowed = FALSE;
}

/*char TabBarClass::FarTabShortcut(int tabIndex)
{
	return tabIndex < 10 ? '0' + tabIndex : 'A' + tabIndex - 10;
}*/

void TabBarClass::FarSendChangeTab(int tabIndex)
{
	SetWindowLong(ghWndDC, 0, tabIndex);
	//PostMessage(ghConWnd, WM_KEYDOWN, VK_F14, 0);
	//PostMessage(ghConWnd, WM_KEYUP, VK_F14, 0);
	//PostMessage(ghConWnd, WM_KEYDOWN, FarTabShortcut(tabIndex), 0);
	//PostMessage(ghConWnd, WM_KEYUP, FarTabShortcut(tabIndex), 0);

	CConEmuPipe pipe;
	if (pipe.Init(_T("TabBarClass::FarSendChangeTab")))
	{
		DWORD cbWritten = 0;
		if (pipe.Execute(CMD_SETWINDOW))
		{
			gConEmu.DnDstep(_T("Tab: Checking for plugin (1 sec)"));
			// �������� ��������, �������� ��� ������ �����
			cbWritten = WaitForSingleObject(pipe.hEventAlive, CONEMUALIVETIMEOUT);
			if (cbWritten!=WAIT_OBJECT_0) {
				TCHAR szErr[MAX_PATH];
				wsprintf(szErr, _T("ConEmu plugin is not active!\r\nProcessID=%i"), pipe.nPID);
				MBoxA(szErr);
			} else {
				gConEmu.DnDstep(_T("Tab: Waiting for result (10 sec)"));
				cbWritten = WaitForSingleObject(pipe.hEventReady, CONEMUREADYTIMEOUT);
				if (cbWritten!=WAIT_OBJECT_0) {
					TCHAR szErr[MAX_PATH];
					wsprintf(szErr, _T("Command waiting time exceeds!\r\nConEmu plugin is locked?\r\nProcessID=%i"), pipe.nPID);
					MBoxA(szErr);
				} else {
					gConEmu.DnDstep(_T("Tab: Recieving data"));
					DWORD cbBytesRead=0;
					int nWholeSize=0;
					pipe.Read(&nWholeSize, sizeof(nWholeSize), &cbBytesRead); 

					#pragma message("TODO: ��������� ����� ����� - ��� COPYDATASTRUCT, ���� nWholeSize")
					gConEmu.DnDstep(NULL);

					pipe.Close();

					Retrieve();//TODO: ������ �� ���-�� ��������������...
				}
			}
		}
	}
}

LRESULT CALLBACK TabBarClass::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		{
			TabBar.OnMouse(uMsg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		}
	}
	return CallWindowProc(_defaultTabProc, hwnd, uMsg, wParam, lParam);
}


bool TabBarClass::IsActive()
{
	return _active;
}

bool TabBarClass::IsShown()
{
	return _active && IsWindowVisible(_hwndTab);
}

/*int TabBarClass::Height()
{
	return _tabHeight;
}*/

BOOL TabBarClass::IsAllowed()
{
	BOOL lbTabsAllowed = TRUE;
	if (gConEmu.BufferHeight) {
        CONSOLE_SCREEN_BUFFER_INFO inf; memset(&inf, 0, sizeof(inf));
		CVirtualConsole* pCon = gConEmu.ActiveCon();
		if (!pCon) return FALSE;
        GetConsoleScreenBufferInfo(pCon->hConOut(), &inf);
        if (inf.dwSize.Y>(inf.srWindow.Bottom-inf.srWindow.Top+1))
			lbTabsAllowed = FALSE;
	}
	return lbTabsAllowed;
}

void TabBarClass::Activate()
{
	if (!_hwndTab) {
		RECT rcClient; 
		GetClientRect(ghWnd, &rcClient); 
		InitCommonControls(); 
		DWORD nPlacement = TCS_SINGLELINE;
		_hwndTab = CreateWindow(WC_TABCONTROL, NULL, nPlacement | WS_CHILD | WS_CLIPSIBLINGS | TCS_FOCUSNEVER, 0, 0, 
			rcClient.right, 0, ghWnd, NULL, g_hInstance, NULL);
		if (_hwndTab == NULL)
		{ 
			return; 
		}
		HFONT hFont = CreateFont(TAB_FONT_HEIGTH, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TAB_FONT_FACE);
		SendMessage(_hwndTab, WM_SETFONT, WPARAM (hFont), TRUE);
		
		#pragma warning (disable : 4312)
		_defaultTabProc = (WNDPROC)SetWindowLongPtr(_hwndTab, GWL_WNDPROC, (LONG_PTR)TabProc);

		if (gConEmu.mh_ConMan)
			CreateToolbar();
	}

	_active = true;
}

void TabBarClass::Deactivate()
{
	if (!_active /*|| !_tabHeight*/)
		return;

	_tabHeight = 0; memset(&m_Margins, 0, sizeof(m_Margins));
	UpdatePosition();
	_active = false;
}

void TabBarClass::Update(ConEmuTab* tabs, int tabsCount)
{
	if (!_active)
	{
		return;
	}
	
#ifdef _DEBUG
	WCHAR szDbg[128]; swprintf(szDbg, L"TabBarClass::Update(%i)\n", tabsCount);
	OutputDebugStringW(szDbg);
#endif

	int i;
	//TabCtrl_DeleteAllItems(_hwndTab);
	for (i = 0; i < tabsCount; i++)
	{
		// get file name
		TCHAR dummy[MAX_PATH*2];
		TCHAR fileName[MAX_PATH+4];
		TCHAR szFormat[32];
		wchar_t* tFileName=NULL; //--Maximus
		if (tabs[i].Name[0]==0 || tabs[i].Type == 1/*WTYPE_PANELS*/) {
			_tcscpy(tabs[i].Name, gConEmu.isFar() ? gSet.szTabPanels : gSet.pszTabConsole);
			tFileName = tabs[i].Name;
			_tcscpy(szFormat, _T("%s"));
		} else {
			GetFullPathName(tabs[i].Name, MAX_PATH*2, dummy, &tFileName);
			if (!tFileName)
				tFileName = tabs[i].Name;

			if (tabs[i].Type == 3/*WTYPE_EDITOR*/) {
				if (tabs[i].Modified)
					_tcscpy(szFormat, gSet.szTabEditorModified);
				else
					_tcscpy(szFormat, gSet.szTabEditor);
			} 
			else if (tabs[i].Type == 2/*WTYPE_VIEWER*/)
				_tcscpy(szFormat, gSet.szTabViewer);
		}
		// restrict length
		int origLength = _tcslen(tFileName);
		int nMaxLen = gSet.nTabLenMax - _tcslen(szFormat) + 2/* %s */;
		if (nMaxLen<15) nMaxLen=15; else
			if (nMaxLen>=MAX_PATH) nMaxLen=MAX_PATH-1;
		if (origLength > nMaxLen)
		{
			/*_tcsnset(fileName, _T('\0'), MAX_PATH);
			_tcsncat(fileName, tFileName, 10);
			_tcsncat(fileName, _T("..."), 3);
			_tcsncat(fileName, tFileName + origLength - 10, 10);*/
			int nSplit = nMaxLen*2/3;
			TCHAR szEllip[MAX_PATH+1];
			_tcsncpy(szEllip, tFileName, nSplit); szEllip[nSplit]=0;
			_tcscat(szEllip, _T("..."));
			_tcscat(szEllip, tFileName + origLength - (nMaxLen - nSplit));
			wsprintf(fileName, szFormat, szEllip);
		} else {
			wsprintf(fileName, szFormat, tFileName);
		}

		AddTab(fileName, i);
	}
	
	// ������� ������ ��������
	int nCurCount = TabCtrl_GetItemCount(_hwndTab);
	for (i=tabsCount; i<nCurCount; i++)
		TabCtrl_DeleteItem(_hwndTab, i);

	if (tabsCount) {
		int ncur = 0;
		for (i = tabsCount-1; i >= 0; i--)
		{
			if (tabs[i].Current)
			{
				ncur = i; break;
			}
		}
		SelectTab(ncur);
	}
	
	if (_tabHeight && tabsCount==1 && tabs[0].Type == 1/*WTYPE_PANELS*/ && gSet.isTabs==2) {
		// ����������� ����� (��� ���������/������� �������)
		Deactivate();
	} else
	if (_tabHeight == NULL)
	{
		RECT rcClient, rcWnd;
		GetClientRect(ghWnd, &rcClient); 
		rcWnd = rcClient;
		TabCtrl_AdjustRect(_hwndTab, FALSE, &rcClient);
		_tabHeight = rcClient.top;

		m_Margins.top = rcClient.top;
		if (gSet.isTabFrame)
		{
			m_Margins.left = rcClient.left;
			m_Margins.right = rcWnd.right-rcClient.right;
			m_Margins.bottom = rcWnd.bottom-rcClient.bottom;
		}
		gSet.UpdateMargins(m_Margins);

		UpdatePosition();
	}
}

RECT TabBarClass::GetMargins()
{
	return m_Margins;
}

void TabBarClass::UpdatePosition()
{
	if (!_active)
	{
		return;
	}
	RECT client, self;
	GetClientRect(ghWnd, &client);
	GetWindowRect(_hwndTab, &self);
	
	gConEmu.ReSize();
	
	if (_tabHeight>0) {
		ShowWindow(_hwndTab, SW_SHOW);
		if (gSet.isTabFrame)
			MoveWindow(_hwndTab, 0, 0, client.right, client.bottom, 1);
		else
			MoveWindow(_hwndTab, 0, 0, client.right, _tabHeight, 1);
	} else {
		ShowWindow(_hwndTab, SW_HIDE);
	}
}

void TabBarClass::UpdateWidth()
{
	if (!_active)
	{
		return;
	}
	RECT client, self;
	GetClientRect(ghWnd, &client);
	GetWindowRect(_hwndTab, &self);
	if (gSet.isTabFrame) {
		MoveWindow(_hwndTab, 0, 0, client.right, client.bottom, 1);
	} else {
		MoveWindow(_hwndTab, 0, 0, client.right, _tabHeight, 1);
	}

	UpdateToolbarPos();
}

void TabBarClass::UpdateToolbarPos()
{
	if (mh_ToolbarParent) {
		//SetWindowPos(mh_ToolbarParent, HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
		   SIZE sz; 
		   SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
		   RECT rcClient;
		   GetWindowRect(_hwndTab, &rcClient);
		   MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcClient, 2);
		   //int nbWidth = SendMessage(mh_Toolbar, TB_GETBUTTONSIZE, 0, 0) & 0xFFFF;
		   //int nHidden = 10;
		   SetWindowPos(mh_ToolbarParent, HWND_TOP, 
			   rcClient.right - sz.cx - 2, 0,
			   sz.cx, sz.cy, 0);
	}
}

bool TabBarClass::OnNotify(LPNMHDR nmhdr)
{
	if (!_active)
	{
		return false;
	}

	//SetFocus(ghWnd); // 02.04.2009 Maks - ?
	if (nmhdr->code == TCN_SELCHANGING)
	{
		//if (mb_ChangeAllowed) {
		//	return FALSE;
		//}
		_prevTab = TabCtrl_GetCurSel(_hwndTab); 
		return FALSE; // ���������
	}

	if (nmhdr->code == TCN_SELCHANGE)
	{
		int lnNewTab = TabCtrl_GetCurSel(_hwndTab);
		//_tcscpy(_lastTitle, gConEmu.Title);
		
		if (_prevTab>=0) {
			SelectTab(_prevTab);
			_prevTab = -1;
		}
		
		if (mb_ChangeAllowed) {
			return FALSE;
		}
		
		FarSendChangeTab(lnNewTab);
		// start waiting for title to change
		_titleShouldChange = true;
		return true;
	} 
	return false;
}



void TabBarClass::OnTimer()
{
	//if (!_active)
	//{
	//	return;
	//}
	//if (_titleShouldChange)
	//{
	//	// title hasn't changed - tab switching was unsuccessful - return to the previous selected tab
	//	if (_tcscmp(_lastTitle, gConEmu.Title) == 0)
	//	{
	//		SelectTab(_prevTab);
	//	}
	//	_titleShouldChange = false;
	//}
	return;
}

void TabBarClass::OnMouse(int message, int x, int y)
{
	if (!_active)
	{
		return;
	}

	//SetFocus(ghWnd);
	if (message == WM_MBUTTONUP || message == WM_RBUTTONUP)
	{
		TCHITTESTINFO htInfo;
		htInfo.pt.x = x;
		htInfo.pt.y = y;
		int iPage = TabCtrl_HitTest(_hwndTab, &htInfo);
		if (iPage != -1)
		{
			FarSendChangeTab(iPage);
			Sleep(50); // TODO
			PostMessage(ghConWnd, WM_KEYDOWN, VK_F10, 0);
			PostMessage(ghConWnd, WM_KEYUP, VK_F10, (LPARAM)(3<<30));
		}
	}
}

void TabBarClass::Invalidate()
{
	if (TabBar.IsActive())
		InvalidateRect(_hwndTab, NULL, TRUE);
}

void TabBarClass::OnConman(int nConNumber, BOOL bAlternative)
{
	if (!mh_Toolbar) return;

	if (gConEmu.ConMan_ProcessCommand) {
		int nGrpCount = gConEmu.ConMan_ProcessCommand(11/*GET_STATUS*/,0,0);
		FarTitle title; memset(&title, 0, sizeof(title));
		BOOL bPresent[MAX_CONSOLE_COUNT]; memset(bPresent, 0, sizeof(bPresent));
		MCHKHEAP
		for (int i=1; i<=nGrpCount; i++) {
			if (gConEmu.ConMan_ProcessCommand(45/*GET_TITLEBYIDX*/,i,(int)&title)) {
				if (title.num>=1 && title.num<=MAX_CONSOLE_COUNT)
					bPresent[title.num-1] = TRUE;
			}
		}

		SendMessage(mh_Toolbar, WM_SETREDRAW, 0, 0);
		for (int i=1; i<=MAX_CONSOLE_COUNT; i++) {
			SendMessage(mh_Toolbar, TB_HIDEBUTTON, i, !bPresent[i-1]);
		}

		UpdateToolbarPos();
		SendMessage(mh_Toolbar, WM_SETREDRAW, 1, 0);

		nConNumber = gConEmu.ConMan_ProcessCommand(46/*GET_ACTIVENUM*/,0,0);
	}
	
	if (nConNumber>=1 && nConNumber<=MAX_CONSOLE_COUNT) {
		SendMessage(mh_Toolbar, TB_CHECKBUTTON, nConNumber, 1);
	} else {
		for (int i=1; i<=MAX_CONSOLE_COUNT; i++)
			SendMessage(mh_Toolbar, TB_CHECKBUTTON, i, 0);
	}
	SendMessage(mh_Toolbar, TB_CHECKBUTTON, 14, bAlternative);
}

LRESULT TabBarClass::ToolWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	switch (messg) {
		case WM_NOTIFY:
			{
				LPNMHDR pHdr = (LPNMHDR)lParam;
				switch (pHdr->code) {
					case TBN_GETINFOTIP:
					{
						if (!gConEmu.isConman() || !gConEmu.ConMan_ProcessCommand)
							break;
						LPNMTBGETINFOTIP pDisp = (LPNMTBGETINFOTIP)lParam;
						if (pDisp->iItem>=1 && pDisp->iItem<=MAX_CONSOLE_COUNT) {
							if (!pDisp->pszText || !pDisp->cchTextMax) break;
							FarTitle title; memset(&title, 0, sizeof(title));
							if (gConEmu.ConMan_ProcessCommand(44/*GET_TITLEBYNUM*/,pDisp->iItem,(int)&title)) {
								lstrcpyn(pDisp->pszText, title.title, pDisp->cchTextMax);
							}
						} else
						if (pDisp->iItem==13) {
							lstrcpyn(pDisp->pszText, _T("Create new console"), pDisp->cchTextMax);
						} else
						if (pDisp->iItem==14) {
							lstrcpyn(pDisp->pszText, _T("Alternative console"), pDisp->cchTextMax);
						}
					}
					break;
				}
			}
			break;
		case WM_COMMAND:
			{
				if (!gConEmu.isConman() || !gConEmu.mh_ConMan || gConEmu.mh_ConMan==INVALID_HANDLE_VALUE)
					break;

				//if (!TabBar.GetConsolesTitles)
				//	TabBar.GetConsolesTitles =
				//		(GetConsolesTitles_t*)GetProcAddress( gConEmu.mh_Infis, "GetConsolesTitles" );
				//if (!TabBar.ActivateConsole)
				//	TabBar.ActivateConsole =
				//		(ActivateConsole_t*)GetProcAddress( gConEmu.mh_Infis, "ActivateConsole" );
				if (!TabBar.ConMan_KeyAction)
					TabBar.ConMan_KeyAction = (ConMan_KeyAction_t)GetProcAddress( gConEmu.mh_ConMan, "_KeyAction_" );
				if (!TabBar.ConMan_KeyAction)
					break;

				RegShortcut cmd; memset(&cmd, 0, sizeof(cmd));
				if (wParam>=1 && wParam<=MAX_CONSOLE_COUNT)
				{
					// ������������ ������� �
					cmd.action = wParam - 1;
					gConEmu.mb_IgnoreSizeChange = true;
					CVirtualConsole* pCon = gConEmu.ActiveCon();
					if (pCon) {
						COORD sz = {pCon->TextWidth, pCon->TextHeight};
						TabBar.ConMan_KeyAction ( &cmd );
						// ���������� ������ �������!
						gConEmu.SetConsoleWindowSize(sz, false);
						gConEmu.mb_IgnoreSizeChange = false;
					}
				} else
				if (wParam==13)
				{
					// ������� ����� �������
					cmd.action = 20;
					TabBar.ConMan_KeyAction ( &cmd );
				} else
				if (wParam==14)
				{
					// ������������ ����� �������������� ��������
					cmd.action = 15;
					TabBar.ConMan_KeyAction ( &cmd );
				}
			}
			break;
	}
	return DefWindowProc(hWnd, messg, wParam, lParam);
}

void TabBarClass::CreateToolbar()
{
	if (!_hwndTab)
		return; // ��� ����� - ��� � �������
	if (mh_Toolbar)
		return; // ��� �������
	
	WNDCLASS wc = {CS_DBLCLKS, (WNDPROC)TabBarClass::ToolWndProc, 0, 0, 
		    g_hInstance, 0, LoadCursor(NULL, IDC_ARROW), 
		    (HBRUSH)(COLOR_BTNFACE+1), 
		    NULL, _T("VirtualConsoleClassConMan")};
    if (!RegisterClass(&wc))
        return;

	
	mh_ToolbarParent = CreateWindow(_T("VirtualConsoleClassConMan"), _T(""), 
			WS_VISIBLE|WS_CHILD, 300,0,340,22, ghWnd, 0, 0, 0);
	if (!mh_ToolbarParent) return;
	
   INITCOMMONCONTROLSEX icex;
    
// Ensure that the common control DLL is loaded. 
   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC  = ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);
 
// Create a toolbar. 
	TBBUTTON buttons[16] = {
		{0, 1, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP},
		{1, 2, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP},
		{2, 3, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{3, 4, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{4, 5, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{5, 6, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{6, 7, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{7, 8, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{8, 9, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{9, 10, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{10, 11, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{11, 12, TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECKGROUP/*|TBSTYLE_TRANSPARENT*/},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
		{12, 13, TBSTATE_ENABLED, BTNS_BUTTON},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
		{13, 14, TBSTATE_ENABLED, TBSTYLE_CHECK}
		};

	mh_Toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
        WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT|CCS_NODIVIDER|TBSTYLE_TOOLTIPS, 0, 0, 0, 0, mh_ToolbarParent, 
        NULL, NULL, NULL); 
 
   SendMessage(mh_Toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
   SendMessage(mh_Toolbar, TB_SETBITMAPSIZE, 0, MAKELONG(16,16)); 
   TBADDBITMAP bmp = {g_hInstance,IDB_CONMAN1};
   int nFirst = SendMessage(mh_Toolbar, TB_ADDBITMAP, 14, (LPARAM)&bmp);
   //buttons
   SendMessage(mh_Toolbar, TB_ADDBUTTONS, 16, (LPARAM)&buttons);

   SendMessage(mh_Toolbar, TB_AUTOSIZE, 0, 0); 
   SIZE sz; 
   SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
   RECT rcClient;
   GetWindowRect(_hwndTab, &rcClient);
   MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcClient, 2);
   //int nbWidth = SendMessage(mh_Toolbar, TB_GETBUTTONSIZE, 0, 0) & 0xFFFF;
   //int nHidden = 10;
   SetWindowPos(mh_ToolbarParent, HWND_TOP, 
	   rcClient.right - sz.cx - 2, 0,
	   sz.cx, sz.cy, 0);

   if (gConEmu.ConMan_ProcessCommand)
	   OnConman(0, FALSE);

   //ShowWindow(hwndTB, SW_SHOW); 
   return;
}
