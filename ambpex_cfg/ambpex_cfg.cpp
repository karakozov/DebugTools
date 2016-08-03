// ambpex_cfg.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include <commctrl.h>
#pragma comment(lib, "comctl32")

#include <shlobj.h>

#include "ambpex_cfg.h"
#include "drvfunc.h"
#include "devfunc.h"
#include "icr.h"

#define MAX_LOADSTRING 100

// Global Variables:
int g_dev;
HWND g_hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
int g_submFlg[3];
int g_basemFlg;
int g_drvFlg;
int g_DmaChanVerFlg;
int g_tetrFlg;
USHORT g_DevID = 0;
TCHAR g_szDir[MAX_LOADSTRING];

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	MainDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	g_dev = 0;
	if(lstrlen(lpCmdLine))
	{
		g_dev = _ttoi(lpCmdLine);
		_tcscpy(g_szDir, lpCmdLine + 2*sizeof(TCHAR) );
		SetCurrentDirectory(g_szDir);
	}

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_AMBPEX_CFG, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	InitCommonControls();

	DialogBox(hInstance, (LPCTSTR)IDD_AMBPAGE, g_hWnd, (DLGPROC)MainDlgProc);
	//HWND hwndMain = CreateDialog(hInstance, (LPCTSTR)IDD_AMBPAGE, g_hWnd, (DLGPROC)MainDlgProc);
    //ShowWindow(hwndMain, SW_SHOW); 
    //UpdateWindow(hwndMain); 

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDI_AMBICON));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//		if(!hwndMain || !IsDialogMessage(hwndMain, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AMBICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_AMBPEX_CFG);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AMBICON));
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   g_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hWnd)
   {
      return FALSE;
   }

//   ShowWindow(g_hWnd, nCmdShow);
//   UpdateWindow(g_hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//***************************************************************************************
void DisplayTrdList(HWND hList, TRD_INFO* info)
{
	LVCOLUMN LvCol; // Make Column struct for ListView
    memset(&LvCol,0,sizeof(LvCol)); // Reset column
	LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM; // Type of mask
	LvCol.cx=0x15;                                // width between each column
	LvCol.pszText = _T("#");                     // First Header

	int sss = sizeof(LVCOLUMN);
	// Inserting column as much as we want
	SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol); // Insert/Show the column
	LvCol.cx=0x27;
	LvCol.pszText = _T("ID");                          // Next column
    SendMessage(hList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol); // ...
	LvCol.cx=0x74;
	LvCol.pszText = _T("Name");                       //
    SendMessage(hList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol); //
	LvCol.cx=0x25;
	LvCol.pszText = _T("MOD");                              //
    SendMessage(hList,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol); //
	LvCol.cx=0x23;
	LvCol.pszText = _T("VER");                            //
    SendMessage(hList,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol); //
	LvCol.cx=0x56;
	LvCol.pszText = _T("FIFO");                      //
    SendMessage(hList,LVM_INSERTCOLUMN,5,(LPARAM)&LvCol); // ...same as above
	LvCol.cx=0x37;
	LvCol.pszText = _T("STATUS");                      //
    SendMessage(hList,LVM_INSERTCOLUMN,6,(LPARAM)&LvCol); // ...same as above

	//SendMessage(hList,LVM_SETTEXTCOLOR, 0,(LPARAM)RGB(0, 128, 128));

	LVITEM LvItem;  // ListView Item struct
	TCHAR nstr[MAX_STRING_LEN];
	TCHAR buf[64];
	int j = 0;
	for(int iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		memset(&LvItem,0,sizeof(LvItem)); // Reset Item Struct
		LvItem.mask=LVIF_TEXT;   // Text Style
		LvItem.cchTextMax = 256; // Max size of test
		TetradName(info[iTetr].id, nstr);
		if(info[iTetr].id && info[iTetr].id != 0xFFFF)
		{
			if(info[iTetr].id == 1 && iTetr) // 2-ой тетрады MAIN быть не может
				break;

			// номер тетрады
			LvItem.iItem = j++;          // choose item  
			LvItem.iSubItem = 0;       // Put in first coluom
			_stprintf(buf, _T("%d"), iTetr);
			LvItem.pszText = buf;  // Text to display (can be from a char variable) (Items)
			SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&LvItem); // Send to the Listview
			
			// ID тетрады
			LvItem.iSubItem = 1;
			_stprintf(buf,_T("%04X"), info[iTetr].id);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// имя тетрады
			LvItem.iSubItem = 2;
			LvItem.pszText = nstr;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// модификатор тетрады
			LvItem.iSubItem = 3;
			_stprintf(buf,_T("%d"), info[iTetr].mod);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// версия терады
			LvItem.iSubItem = 4;
			_stprintf(buf,_T("%d.%d"), info[iTetr].ver>>8, info[iTetr].ver&0xff);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// FIFO терады
			LvItem.iSubItem = 5;
			if(info[iTetr].tres & 0x10)
			{
				_stprintf(buf,_T("IN %dx%d"), info[iTetr].fsize, info[iTetr].ftype);
				//_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d FIFOIN %dx%d"), 
				//	i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff, info[i].fsize, info[i].ftype);
			}
			else
			{
				if(info[iTetr].tres & 0x20)
				{
					_stprintf(buf,_T("OUT %dx%d"), info[iTetr].fsize, info[iTetr].ftype);
					//_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d FIFOOUT %dx%d"), 
					//	i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff, info[i].fsize, info[i].ftype);
				}
				else
				{
					_stprintf(buf, _T("-"));
					//_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d"), 
					//			i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff);
				}
			}
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// состояние терады
			LvItem.iSubItem = 6;
			_stprintf(buf,_T("%04X"), info[iTetr].status);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems
		}
		else
			continue;
			//  Setting properties Of Items:
	}
}

//***************************************************************************************
void UpdateTrdList(HWND hList, TRD_INFO* info)
{
	LVITEM LvItem;  // ListView Item struct
	TCHAR nstr[MAX_STRING_LEN];
	TCHAR buf[64];
	int j = 0;
	for(int iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		memset(&LvItem,0,sizeof(LvItem)); // Reset Item Struct
		LvItem.mask=LVIF_TEXT;   // Text Style
		LvItem.cchTextMax = 256; // Max size of test
		TetradName(info[iTetr].id, nstr);
		if(info[iTetr].id && info[iTetr].id != 0xFFFF)
		{
			if(info[iTetr].id == 1 && iTetr) // 2-ой тетрады MAIN быть не может
				break;

			LvItem.iItem = j++;          // choose item  
			// ID тетрады
			LvItem.iSubItem = 1;
			_stprintf(buf,_T("%04X"), info[iTetr].id);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// имя тетрады
			LvItem.iSubItem = 2;
			LvItem.pszText = nstr;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// модификатор тетрады
			LvItem.iSubItem = 3;
			_stprintf(buf,_T("%d"), info[iTetr].mod);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// версия терады
			LvItem.iSubItem = 4;
			_stprintf(buf,_T("%d.%d"), info[iTetr].ver>>8, info[iTetr].ver&0xff);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// FIFO терады
			LvItem.iSubItem = 5;
			if(info[iTetr].tres & 0x10)
			{
				_stprintf(buf,_T("IN %dx%d"), info[iTetr].fsize, info[iTetr].ftype);
				//_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d FIFOIN %dx%d"), 
				//	i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff, info[i].fsize, info[i].ftype);
			}
			else
			{
				if(info[iTetr].tres & 0x20)
				{
					_stprintf(buf,_T("OUT %dx%d"), info[iTetr].fsize, info[iTetr].ftype);
					//_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d FIFOOUT %dx%d"), 
					//	i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff, info[i].fsize, info[i].ftype);
				}
				else
				{
					_stprintf(buf, _T("-"));
					//_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d"), 
					//			i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff);
				}
			}
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			// состояние терады
			LvItem.iSubItem = 6;
			_stprintf(buf,_T("%04X"), info[iTetr].status);
			LvItem.pszText = buf;
			SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems
		}
		else
			continue;
			//  Setting properties Of Items:
	}
}

//***************************************************************************************
void ClearTrdList(HWND hList)
{
	LRESULT ret;
	for(int iTetr = MAX_TETRNUM-1; iTetr >= 0; iTetr--)
	{
		SendMessage(hList, LVM_DELETEITEM, iTetr, 0);
	}

	for(int i = 6; i >= 0; i--)
		ret = SendMessage(hList, LVM_DELETECOLUMN, i, NULL);

/*	ret = SendMessage(hList, LVM_DELETECOLUMN, 5, NULL); // Insert/Show the column
	ret = SendMessage(hList, LVM_DELETECOLUMN, 4, NULL); // Insert/Show the column
	ret = SendMessage(hList, LVM_DELETECOLUMN, 3, NULL); // Insert/Show the column
	ret = SendMessage(hList, LVM_DELETECOLUMN, 2, NULL); // Insert/Show the column
	ret = SendMessage(hList, LVM_DELETECOLUMN, 1, NULL); // Insert/Show the column
	ret = SendMessage(hList, LVM_DELETECOLUMN, 0, NULL); // Insert/Show the column
	for(int i = 0; i < 7; i++)
	{
		lResult ret = SendMessage(hList, LVM_DELETECOLUMN, i, NULL); // Insert/Show the column
	}

	LVITEM LvItem;  // ListView Item struct
	int j = 0;
	for(int iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		memset(&LvItem,0,sizeof(LvItem)); // Reset Item Struct
		LvItem.mask=LVIF_TEXT;   // Text Style
		LvItem.cchTextMax = 256; // Max size of test

		// номер тетрады
		LvItem.iItem = iTetr;          // choose item  
		LvItem.iSubItem = 0;       // Put in first coluom
		LvItem.pszText = "";  // Text to display (can be from a char variable) (Items)
		SendMessage(hList, LVM_DELETEITEM, 0, (LPARAM)&LvItem); // Send to the Listview
	}
	*/
}

int CFG_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	//InitToolTips(hDlg);
	g_tetrFlg = 0;
	HDC hDC = GetWindowDC(hdlg);
	SetTextColor(hDC, RGB(0, 128, 128));
	SetBkColor(hDC, RGB(0, 128, 128));
	ULONG status = 0;
	//if(dev_open(g_dev) < 0)
	//{
	//	g_drvFlg = 0;
	//	SetDlgItemText(hdlg, IDC_VERSION, _T("Open error!"));
	//}
	//else
	int err = dev_open(g_dev);
	int fl_pex = 1;
	if(err < 0)
	{
		fl_pex = 0;
		err = dev_openPlx(g_dev);
		if(err < 0)
		{
			g_drvFlg = 0;
			SetDlgItemText(hdlg, IDC_VERSION, _BRDC("Open error!"));
		}
	}

	if(err >= 0)
	{
		//SetDlgItemText(hdlg, IDC_VERSION, L"Open success!");
		TCHAR buf[MAX_STRING_LEN];
		char ver_buf[MAX_STRING_LEN];
		status = GetVersion(ver_buf);
		if(status)
		{
			g_drvFlg = 0;
			_stprintf_s(buf, MAX_STRING_LEN, _T("Error 0x%X"), status);
			SetDlgItemText(hdlg, IDC_VERSION, buf);
		}
		else
		{
			//swprintf(buf, L"Driver %s", buf);
#ifdef _WIN64
			mbstowcs(buf,ver_buf, MAX_STRING_LEN);
#else
			lstrcpy(buf,ver_buf);
#endif
			SetDlgItemText(hdlg, IDC_VERSION, buf);
			g_drvFlg = 1;
			GetDeviceID(g_DevID);
			ULONG rev;
			GetPciCfgReg(rev, 8);
			TCHAR devstr[MAX_STRING_LEN];
			BasemodName(g_DevID, devstr);
			ULONG slot, bus, dev;
			GetLocation(slot, bus, dev);
			if(g_DevID == 0x53B1 || g_DevID == 0x53B3 ||   // FMC115cP or FMC117cP
				g_DevID == 0x5514 || g_DevID == 0x5518 || g_DevID == 0x5519) // AC_SYNC_DEVID or PS_DSP_DEVID or PS_ADC_DEVID
			{
				_stprintf(buf, _T(" %s v%d.%d : bus %d, dev %d, G.adr %d, order %d"), devstr, (rev & 0xff) >> 4, rev & 0xf, 
																	bus, dev, slot & 0xffff, slot >> 16);
			}
			else
			{
				if(slot != (ULONG)-1)
					_stprintf(buf, _T(" %s v%d.%d : slot %d (bus %d, dev %d)"), devstr, (rev & 0xff) >> 4, rev & 0xf, slot, bus, dev);
				else
					_stprintf(buf, _T(" %s v%d.%d : bus %d, dev %d "), devstr, (rev & 0xff) >> 4, rev & 0xf, bus, dev);
			}
			if(g_DevID == 0x551A) // PS_SYNC_DEVID
			{
				_stprintf(buf, _T(" %s v%d.%d : bus %d, dev %d, G.adr %d"), devstr, (rev & 0xff) >> 4, rev & 0xf, 
																		bus, dev, slot & 0xffff);
			}
			SetWindowText(hdlg, buf);
			if(fl_pex)
			{
//				ULONG link_speed = 0;
//				ULONG link_width = LinkWidth(link_speed);
				ULONG pciex_info[4];
				PCIE_info(pciex_info);

				//if(link_speed == 1)
				if(pciex_info[0] == 1)
				{
					SetDlgItemText(hdlg, IDC_LABEL_PCIE, _T("PCI Express 1.1"));
				}
				else
					//if(link_speed == 2)
					if(pciex_info[0] == 2)
					{
						SetDlgItemText(hdlg, IDC_LABEL_PCIE, _T("PCI Express 2.0"));
					}

				//_stprintf(buf, _T("Lanes: %d"), link_width);
				_stprintf(buf, _T("Lanes: %d"), pciex_info[1]);
				SetDlgItemText(hdlg, IDC_LINKWIDTH, buf);

				_stprintf(buf, _T("Max Payload (Supported) = %d (%d) bytes"), pciex_info[2], pciex_info[3]);
				SetDlgItemText(hdlg, IDC_PCIE_PAYLOAD, buf);

				HWND hWnd = GetDlgItem(hdlg, IDC_STPLDNAME);
				EnableWindow(hWnd, FALSE);
				hWnd = GetDlgItem(hdlg, IDC_PLDFILENAME);
				EnableWindow(hWnd, FALSE);
				hWnd = GetDlgItem(hdlg, IDC_PLDFILEBROWSE);
				EnableWindow(hWnd, FALSE);
				hWnd = GetDlgItem(hdlg, IDB_LOAD);
				EnableWindow(hWnd, FALSE);
			}
			else
			{
				if(g_DevID == 0x53A0 || g_DevID == 0x53A1 || g_DevID == 0x53A2)
					 SetDlgItemText(hdlg, IDC_LABEL_PCIE, _T("CompactPCI Bus"));
				else
					if(g_DevID == 0x4D58 || g_DevID == 0x4D44 || g_DevID == 0x1020)
						 SetDlgItemText(hdlg, IDC_LABEL_PCIE, _T("PCI Bus"));
					else
						SetDlgItemText(hdlg, IDC_LABEL_PCIE, _T("PCI Express 1.1"));
				if(g_DevID == 0x4D58 || g_DevID == 0x53A2)
					_stprintf(buf, _T("Width 32 bits"));
				else
					if(g_DevID == 0x4D44 || g_DevID == 0x1020)
						_stprintf(buf, _T("Width 64 bits"));
					else
						_stprintf(buf, _T("Lanes: 1"));
				SetDlgItemText(hdlg, IDC_LINKWIDTH, buf);

				SetDlgItemText(hdlg, IDC_PCIECINFO, _T(""));
				HWND hWnd = GetDlgItem(hdlg, IDC_PCIECINFO);
				EnableWindow(hWnd, FALSE);
				SetDlgItemText(hdlg, IDC_PCIECINFO2, _T(""));
				hWnd = GetDlgItem(hdlg, IDC_PCIECINFO2);
				EnableWindow(hWnd, FALSE);
				SetDlgItemText(hdlg, IDC_PCIE_PAYLOAD, _T(""));
				hWnd = GetDlgItem(hdlg, IDC_PCIE_PAYLOAD);
				EnableWindow(hWnd, FALSE);

				hWnd = GetDlgItem(hdlg, IDC_LABEL_FMCPOW);
				EnableWindow(hWnd, FALSE);
				hWnd = GetDlgItem(hdlg, IDC_VOLT);
				EnableWindow(hWnd, FALSE);
				hWnd = GetDlgItem(hdlg, IDB_POWER);
				EnableWindow(hWnd, FALSE);
				hWnd = GetDlgItem(hdlg, IDC_POWSTAT);
				EnableWindow(hWnd, FALSE);
			}

			UCHAR eeprom_data[256];
			for(int i = 0; i < 256; i++)
				eeprom_data[i] = 0;
			//WriteNvRAM(eeprom_data, 256, 0x80);
			//for(int i = 0; i < 256; i++)
			if(g_DevID != 0x5520)
				ReadNvRAM(eeprom_data, 256, 0x80);
			USHORT btag = *(USHORT*)eeprom_data;
			TCHAR pldDescrBuf[MAX_STRING_LEN];
			if(btag == 0x4953)
			{
				TCHAR typestr[MAX_STRING_LEN];
				ULONG ser_num = *(ULONG*)(eeprom_data+6);
				ULONG base_type = *(USHORT*)(eeprom_data+10);
				ULONG base_ver = *(eeprom_data+12);
				BasemodName(base_type, typestr);
				_stprintf(buf, _T("%s (0x%04X) : s/n = %d, version = %d.%d"), typestr, base_type, ser_num, base_ver>>4, base_ver&0xF);
				SetDlgItemText(hdlg, IDC_BASESN, buf);
				GetPldDescription(pldDescrBuf, eeprom_data, 256);
				SetDlgItemText(hdlg, IDC_PLDDESCR, pldDescrBuf);
				g_basemFlg = 1;
			}
			else
			{
				SetDlgItemText(hdlg, IDC_BASESN, _T("Base ICR error!!!"));
				_stprintf(pldDescrBuf, _T("Base ICR error!!!"));
				g_basemFlg = 0;
			}
//		dev_close();
//	return TRUE;
			ULONG offset_admicr = 0;
			int subm_present[3];
			if(fl_pex)
			{
				PCIEC_INFO pcic_info;
				if(g_DevID != 0x5520)
					status = HostReg(&pcic_info);
				else
					status = 1;
				if(!status)
				{
					_stprintf(buf, _T("Controller: %d.%d"), 
						pcic_info.pldver >> 8, pcic_info.pldver & 0xff);
					SetDlgItemText(hdlg, IDC_PCIECINFO, buf);
					ULONG extdma_sup = (pcic_info.dmaver & 0xF0000000) >> 28;
					if(extdma_sup)
						_stprintf(buf, _T("DMA channel (Ext): %d.%02d"), 
							(pcic_info.dmaver & 0xFFFF) >> 8, pcic_info.dmaver & 0xff);
					else
						_stprintf(buf, _T("DMA channel: %d.%02d"), 
							(pcic_info.dmaver & 0xFFFF) >> 8, pcic_info.dmaver & 0xff);
					SetDlgItemText(hdlg, IDC_PCIECINFO2, buf);
					g_DmaChanVerFlg = (pcic_info.dmaver < 0x103) ? 0 : 1;
					_stprintf(buf, _T("Core ID: 0x%04X, Core MOD: 0x%04X"), 
						pcic_info.coreid, pcic_info.coremod);
					SetDlgItemText(hdlg, IDC_CORE, buf);
					ULONG irqminper_sup = (pcic_info.irqmin & 0xFF00) >> 8;
					if(irqminper_sup == 0xB5)
					{
						double val = double(pcic_info.irqmin & 0xff);
						_stprintf(buf, _T("IRQ min period : %d mcs"), (ULONG)pow(2., val));
					}
					else
						_stprintf(buf, _T("IRQ min period = 1 ms"));
					SetDlgItemText(hdlg, IDC_IRQMIN, buf);

				}
				else
				{
					_stprintf(buf, _T(""));
					SetDlgItemText(hdlg, IDC_PCIECINFO, buf);
					SetDlgItemText(hdlg, IDC_PCIECINFO2, buf);
					SetDlgItemText(hdlg, IDC_CORE, buf);
					SetDlgItemText(hdlg, IDC_IRQMIN, buf);
				}
				FMC_POWER power;
				if(g_DevID != 0x5520)
					status = GetPower(&power);
				else
				{
					status = 1;
					HWND hWnd = GetDlgItem(hdlg, IDB_TEST);
					EnableWindow(hWnd, FALSE);
					hWnd = GetDlgItem(hdlg, IDB_GRST);
					EnableWindow(hWnd, FALSE);
					hWnd = GetDlgItem(hdlg, IDB_UPD);
					EnableWindow(hWnd, FALSE);
					hWnd = GetDlgItem(hdlg, IDB_ERASEICR);
					EnableWindow(hWnd, FALSE);
				}
				if(!status)
				{
					_stprintf(buf, _T("%.2f Volt"), power.value/100.);
					SetDlgItemText(hdlg, IDC_VOLT, buf);
					if(power.onOff)
					{
						SetDlgItemText(hdlg, IDB_POWER, _T("Off"));
						SetDlgItemText(hdlg, IDC_POWSTAT, _T("Power is On"));
					}
					else
					{
						SetDlgItemText(hdlg, IDB_POWER, _T("On"));
						SetDlgItemText(hdlg, IDC_POWSTAT, _T("Power is Off"));
					}
					offset_admicr = 0x400;
				}
				else
				{
					SetDlgItemText(hdlg, IDC_LABEL_FMCPOW, _T("There is no FMC Power"));
					HWND hWnd = GetDlgItem(hdlg, IDC_VOLT);
					EnableWindow(hWnd, FALSE);
					hWnd = GetDlgItem(hdlg, IDB_POWER);
					EnableWindow(hWnd, FALSE);
					hWnd = GetDlgItem(hdlg, IDC_POWSTAT);
					EnableWindow(hWnd, FALSE);
				}
				if(g_DevID != 0x5520)
					FM_Present(&pcic_info, subm_present);
				else
				{
					subm_present[0] = 0;
					subm_present[1] = 0;
					subm_present[2] = 0;
				}
			}
			else
			{
				if(g_DevID == 0x4D58  || g_DevID == 0x53A2) // AMBPCX, Sync-cP6 
					_stprintf(buf, _T("Controller: PLX PCI9056"));
				if(g_DevID == 0x4D44 || g_DevID == 0x1020) // AMBPCD, ADS10x2G
					_stprintf(buf, _T("Controller: PLX PCI9656"));
				if(g_DevID == 0x5502 || g_DevID == 0x5508 || g_DevID == 0x551B) // AMBPEX1, AMBPEX2, Sync8P
					_stprintf(buf, _T("Controller: PLX PEX8311"));
				else
					_stprintf(buf, _T("Controller: PLX PCI9656"));
				//SetDlgItemText(hdlg, IDC_PCIECINFO, buf);
				SetDlgItemText(hdlg, IDC_CORE, buf);
				subm_present[0] = 1;
				subm_present[1] = 0;
				subm_present[2] = 0;
			}
			if (subm_present[0])
			{
				for (int i = 0; i < 32; i++)
					eeprom_data[i] = 0;
				if(fl_pex)
					ReadSubICR(0, eeprom_data, 32, offset_admicr);
				else
					ReadAdmIdROM(eeprom_data, 32, offset_admicr);
				USHORT admtag = *(USHORT*)eeprom_data;
				if (admtag == 0x0080)
				{
					TCHAR typestr[MAX_STRING_LEN];
					ULONG adm_num = *(ULONG*)(eeprom_data + 7);
					USHORT adm_type = *(USHORT*)(eeprom_data + 11);
					UCHAR adm_ver = *(UCHAR*)(eeprom_data + 13);
					SubmodName(adm_type, typestr);
					_stprintf(buf, _T("Subm %s (0x%04X) : s/n = %d, version = %d.%d"),
						typestr, adm_type, adm_num, adm_ver >> 4, adm_ver & 0xF);
					SetDlgItemText(hdlg, IDC_ADMSN, buf);
					g_submFlg[0] = 1;
				}
				else
				{
					SetDlgItemText(hdlg, IDC_ADMSN, _T("Subm ICR is NOT detected!!!"));
					g_submFlg[0] = 0;
				}
			}
			else
			{
				SetDlgItemText(hdlg, IDC_ADMSN, _T("FM-subm1 is NOT present!!!"));
				g_submFlg[0] = 0;
			}
			if (subm_present[1])
			{
				for (int i = 0; i < 32; i++)
					eeprom_data[i] = 0;
				ReadSubICR(1, eeprom_data, 32, offset_admicr);
				//ReadAdmIdROM(eeprom_data, 32, offset_admicr);
				USHORT admtag = *(USHORT*)eeprom_data;
				if (admtag == 0x0080)
				{
					TCHAR typestr[MAX_STRING_LEN];
					ULONG adm_num = *(ULONG*)(eeprom_data + 7);
					USHORT adm_type = *(USHORT*)(eeprom_data + 11);
					UCHAR adm_ver = *(UCHAR*)(eeprom_data + 13);
					SubmodName(adm_type, typestr);
					_stprintf(buf, _T("Subm2 %s (0x%04X) : s/n = %d, version = %d.%d"),
						typestr, adm_type, adm_num, adm_ver >> 4, adm_ver & 0xF);
					SetDlgItemText(hdlg, IDC_ADM2SN, buf);
					g_submFlg[1] = 1;
				}
				else
				{
					SetDlgItemText(hdlg, IDC_ADM2SN, _T("Subm2 ICR is NOT detected!!!"));
					g_submFlg[1] = 0;
				}
			}
			else
			{
				SetDlgItemText(hdlg, IDC_ADM2SN, _T("FM-subm2 is NOT present!!!"));
				g_submFlg[1] = 0;
			}
			ULONG PldStatus;
			GetPldStatus(PldStatus, 0);
			if(!fl_pex)
			{ // wambp - платы на PLX
				PldStatus = !PldStatus;
				TCHAR pldname[MAX_STRING_LEN];
				BasemodName(g_DevID, pldname);
				_tcslwr(pldname); // Convert a string to lowercase
				_tcscat(pldname, _T("_*"));
				HWND hWndPldName = GetDlgItem(hdlg, IDC_PLDFILENAME);
				WIN32_FIND_DATA FindFileData;
				HANDLE hFind = FindFirstFile(pldname, &FindFileData);
			    if(hFind != INVALID_HANDLE_VALUE) 
				{
					SendMessage(hWndPldName, CB_ADDSTRING, (WPARAM)0, (LPARAM)FindFileData.cFileName);
				    while (FindNextFile(hFind, &FindFileData) != 0) 
				    {
						SendMessage(hWndPldName, CB_ADDSTRING, (WPARAM)0, (LPARAM)FindFileData.cFileName);
					}
					FindClose(hFind);
				}
				SendMessage(hWndPldName, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

			}
			if(PldStatus)
			{
				//SetDlgItemText(hdlg, IDC_PLDSTATUS, L"ADM PLD is loaded!!!");
				PLD_INFO pld_info;
				status = AdmPldWorkAndCheck(&pld_info);
				if(!status)
				{
					if(pld_info.submif == FMC_VERSION)
						SetDlgItemText(hdlg, IDC_LABEL_PLD, _T("ADMF PLD"));
					//swprintf(buf, L"Version = 0x%X, Modification = 0x%X, Build = 0x%X", 
					_stprintf(buf, _T("Ver: %d.%d, Mod: %d, Build: %d"), 
										pld_info.version >> 8, pld_info.version & 0xff, pld_info.modification, pld_info.build);
					SetDlgItemText(hdlg, IDC_PLDINFO, buf);
					//swprintf(buf, L"PLD description");
					//SetDlgItemText(hdlg, IDC_PLDDESCR, pldDescrBuf);

					TRD_INFO info[MAX_TETRNUM];
					TetradList(info);

					HWND hWndTrdList = GetDlgItem(hdlg, IDC_TRDLIST);

					DisplayTrdList(hWndTrdList, info);
					g_tetrFlg = 1;
/*					TCHAR nstr[MAX_STRING_LEN];
					for(int i = 0; i < MAX_TETRNUM; i++)
					{
						TetradName(info[i].id, nstr);
						if(info[i].id)
						{
							if(info[i].id == 1 && i) // 2-ой тетрады MAIN быть не может
								break;
							if(info[i].tres & 0x10)
								_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d FIFOIN %dx%d"), 
									i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff, info[i].fsize, info[i].ftype);
							else
								if(info[i].tres & 0x20)
									_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d FIFOOUT %dx%d"), 
										i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff, info[i].fsize, info[i].ftype);
								else
									_stprintf(buf, _T("%d: %04X %s mod %d ver %d.%d"), 
												i, info[i].id, nstr, info[i].mod, info[i].ver>>8, info[i].ver&0xff);
						}
						else
							continue;
							//swprintf(buf, L"%d: %04X %s       ", i, info[i].id, nstr);
						//SetDlgItemText(hdlg, IDC_TRD0, buf);
						SendMessage(hWndTrdList, LB_ADDSTRING, (WPARAM)0, (LPARAM)buf);
					}*/
					//SendMessage(hWndTrdList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					
				}
				else
				{
					_stprintf(buf, _T("Error by AdmPldWorkAndCheck() = %d (0x%4X)"), status, pld_info.version);
					SetDlgItemText(hdlg, IDC_PLDINFO, buf);
					SetDlgItemText(hdlg, IDC_PLDDESCR, pldDescrBuf);
				}
				TCHAR str_mem_type[MAX_STRING_LEN];
				ULONG mem_size = GetMemorySize(str_mem_type); // в 32-разрядных словах
				if(mem_size)
				{
					_stprintf(buf, _T("%s  %d MBytes"), str_mem_type, mem_size / 1024 / 256);
					SetDlgItemText(hdlg, IDC_MEMSIZE, buf);
				}
			}
			else
				SetDlgItemText(hdlg, IDC_PLDINFO, _T("ADM PLD is NOT loaded!!!"));
		}
		

/*		ULONG smon_stat = 0;
		if(isSysMon(smon_stat))
		{
			_stprintf(buf, _T("Monitor status: %X"), smon_stat);
			SetDlgItemText(hdlg, IDC_SMONSTAT, buf);
			double max_temp, min_temp;
			double cur_temp = getTemp(max_temp, min_temp);
			_stprintf(buf, _T("Temp: %.3f C (%.3f - %.3f)"), cur_temp, min_temp, max_temp);
			SetDlgItemText(hdlg, IDC_TEMP, buf);

			double max_vcc, min_vcc;
			double cur_vcc = getVccint(max_vcc, min_vcc);
			_stprintf(buf, _T("Vccint: %.3f V (%.3f - %.3f)"), cur_vcc, min_vcc, max_vcc);
			SetDlgItemText(hdlg, IDC_VCCINT, buf);

			cur_vcc = getVccaux(max_vcc, min_vcc);
			_stprintf(buf, _T("Vccaux: %.3f V (%.3f - %.3f)"), cur_vcc, min_vcc, max_vcc);
			SetDlgItemText(hdlg, IDC_VCCAUX, buf);

			double refp, refn;
			getVref(refp, refn);
			_stprintf(buf, _T("Vrefp: %.3f V Vrefn: %.3f V"), refp, refn);
			SetDlgItemText(hdlg, IDC_VREF, buf);
		}
*/
		dev_close();
	}

	HWND hWndTestBtn = GetDlgItem(hdlg, IDB_TEST);
	HDC hDCBtn = GetWindowDC(hWndTestBtn);
	SetTextColor(hDCBtn, RGB(0, 128, 128));
	SetBkColor(hDCBtn, RGB(0, 128, 128));
//	SendMessage(hWndTestBtn, WM_CTLCOLORBTN, 0, (LPARAM)RGB(128, 0, 0)); // Enter text to SubItems
//	SendMessage(hList,LVM_SETTEXTCOLOR, 0,(LPARAM)RGB(0, 128, 128));
	return TRUE;
}

int __stdcall BrowseCallbackProc(
								HWND hwnd,
								UINT uMsg,
								LPARAM lParam,
								LPARAM lpData
								)
{
    switch (uMsg)
	{
        case BFFM_INITIALIZED:
			{
            // При инициализации диалога
            // посылаем сообщение установки каталога
			TCHAR szDir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, szDir);
            //if(lpData!=NULL)
                SendMessage(hwnd, BFFM_SETSELECTION,TRUE, (LPARAM)szDir);
            } break;
        case BFFM_SELCHANGED :
            break;
    }
    return 0;
}

// Message handler for Main Dialog box.
LRESULT CALLBACK MainDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	HANDLE_MSG(hdlg, WM_INITDIALOG, CFG_OnInitDialog);
	//RegisterHotKey(hdlg, IDH_NEXT, MOD_ALT, VK_TAB);
	case WM_CTLCOLORSTATIC:
        // Set the colour of the text
        if ((HWND)lParam == GetDlgItem(hdlg, IDC_VERSION)) 
        {
                // we're about to draw the static
                // set the text colour in (HDC)lParam
                SetBkMode((HDC)wParam,TRANSPARENT);
				if(g_drvFlg)
	                SetTextColor((HDC)wParam, RGB(0,0,0));
				else
					SetTextColor((HDC)wParam, RGB(255,0,0));
                //return (BOOL)CreateSolidBrush (GetSysColor(COLOR_WINDOW+1));
				return (LRESULT) GetSysColorBrush(COLOR_WINDOW+10);
        }
        if ((HWND)lParam == GetDlgItem(hdlg, IDC_PCIECINFO2)) 
        {
                // we're about to draw the static
                // set the text colour in (HDC)lParam
                SetBkMode((HDC)wParam,TRANSPARENT);
				if(g_DmaChanVerFlg)
	                SetTextColor((HDC)wParam, RGB(0,0,0));
				else
					SetTextColor((HDC)wParam, RGB(255,0,0));
                //return (BOOL)CreateSolidBrush (GetSysColor(COLOR_WINDOW+1));
				return (LRESULT) GetSysColorBrush(COLOR_WINDOW+10);
        }

        if ((HWND)lParam == GetDlgItem(hdlg, IDC_BASESN)) 
        {
                // we're about to draw the static
                // set the text colour in (HDC)lParam
                SetBkMode((HDC)wParam,TRANSPARENT);
				if(g_basemFlg)
	                SetTextColor((HDC)wParam, RGB(0,0,255));
				else
					SetTextColor((HDC)wParam, RGB(255,0,0));
                //return (BOOL)CreateSolidBrush (GetSysColor(COLOR_WINDOW+1));
				return (LRESULT) GetSysColorBrush(COLOR_WINDOW+10);
        }
        if ((HWND)lParam == GetDlgItem(hdlg, IDC_ADMSN)) 
        {
                // we're about to draw the static
                // set the text colour in (HDC)lParam
                SetBkMode((HDC)wParam,TRANSPARENT);
				if(g_submFlg[0])
	                SetTextColor((HDC)wParam, RGB(0,0,255));
				else
					SetTextColor((HDC)wParam, RGB(255,0,0));
                //return (BOOL)CreateSolidBrush (GetSysColor(COLOR_WINDOW+1));
				return (LRESULT) GetSysColorBrush(COLOR_WINDOW+10);
        }
        if ((HWND)lParam == GetDlgItem(hdlg, IDC_ADM2SN)) 
        {
                // we're about to draw the static
                // set the text colour in (HDC)lParam
                SetBkMode((HDC)wParam,TRANSPARENT);
				if(g_submFlg[1])
	                SetTextColor((HDC)wParam, RGB(0,0,255));
				else
					SetTextColor((HDC)wParam, RGB(255,0,0));
                //return (BOOL)CreateSolidBrush (GetSysColor(COLOR_WINDOW+1));
				return (LRESULT) GetSysColorBrush(COLOR_WINDOW+10);
        }
/*	case WM_CTLCOLORSTATIC:
		if((HWND)lParam==GetDlgItem(hdlg, IDC_BASESN))
		{
			HDC hDC = GetWindowDC((HWND)lParam);
			//SetBkMode(hDC,TRANSPARENT);
			SetTextColor(hDC,RGB(255,0,0));
			//return (LRESULT) GetSysColorBrush(GetSysColor(COLOR_WINDOW));
			return (LRESULT) GetSysColorBrush(COLOR_WINDOW);
			//return (LRESULT) GetSysColorBrush(RGB(255, 0, 0));
			//return (LRESULT) CreateSolidBrush(RGB(256, 256, 256 ));
		}*/
	break;
	//case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOR:
	{
		HDC hDC = GetWindowDC(hdlg);
		SetTextColor(hDC, RGB(0, 256, 256));
		SetBkColor(hDC, RGB(128, 0, 0));
//		switch(LOWORD(wParam))
//		{
  //  	case IDC_BASESN:
			//HWND hWndBaseTxt = GetDlgItem(hdlg, IDC_BASESN);
			//HDC hDCBase = GetWindowDC(hWndBaseTxt);
			//SetTextColor(hDCBase, RGB(0, 128, 128));
			//SetBkColor(hDCBase, RGB(0, 128, 128));
			//TextOut(hDCBase, 40, 40, buf, strlen(buf));
	//		break;
	//	}
	}
	case WM_KEYDOWN:
	{
        switch(wParam)
		{
		case VK_F1: // F1
			{
				//MessageBox(NULL, _BRDC("Base Module ICR will be ERASED!!!\n Are you sure?"), _BRDC("AMBPEX Config"), MB_YESNO ); 
//				HINSTANCE result = ShellExecute(hWnd, _T("open"), sHelpFileName, NULL, NULL, SW_SHOWNORMAL);
//				//ShellExecute(hWnd, _T("open"), sHelpFileName, NULL, NULL, SW_SHOWNORMAL);
				//DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hDlg, (DLGPROC)About);
			}
			break;
		//case VK_DELETE: // 
		//	DelCurLink(hDlg);
		//	break;
		}
		break;
	}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
    	case IDB_LOAD:
    		{					// IDB_LOAD
				HWND hWndLoadBtn = GetDlgItem(hdlg, IDB_LOAD);
				EnableWindow(hWndLoadBtn, FALSE);
				if(dev_openPlx(g_dev) < 0)
				{
					MessageBox(NULL, _BRDC("Device open error!!!"), _BRDC("AMBPEX Config"), MB_OK); 
				}
				else
				{
					TCHAR szFile[MAX_PATH] = _BRDC(""); // путь и имя файла
					GetDlgItemText(hdlg, IDC_PLDFILENAME, szFile, MAX_PATH);
					PVOID fileBuffer = NULL;
					ULONG fileSize;
					int ret = ReadPldFile(szFile, fileBuffer, fileSize);
					if(ret >= 0)
					{
						//memset(info, 0, sizeof(TRD_INFO)*MAX_TETRNUM); 
						ULONG PldStatus;
						ret = LoadPld(PldStatus, fileBuffer, fileSize, 0);
						VirtualFree(fileBuffer, 0, MEM_RELEASE);
						if(PldStatus == 3)
						{
							SetDlgItemText(hdlg, IDC_PLDINFO, _T("ADM PLD is NOT loaded!!!"));
							HWND hWndTrdList = GetDlgItem(hdlg, IDC_TRDLIST);
							ClearTrdList(hWndTrdList);
							g_tetrFlg = 0;
						}
						else
						{
							GetPldStatus(PldStatus, 0);
							PldStatus = !PldStatus;
							if(PldStatus)
							{
								TCHAR buf[MAX_STRING_LEN];
								//SetDlgItemText(hdlg, IDC_PLDSTATUS, L"ADM PLD is loaded!!!");
								PLD_INFO pld_info;
								ret = AdmPldWorkAndCheck(&pld_info);
								if(!ret)
								{
									_stprintf(buf, _T("Ver: %d.%d, Mod: %d, Build: %d"), 
														pld_info.version >> 8, pld_info.version & 0xff, pld_info.modification, pld_info.build);
									SetDlgItemText(hdlg, IDC_PLDINFO, buf);
									TRD_INFO info[MAX_TETRNUM];
									TetradList(info);
									HWND hWndTrdList = GetDlgItem(hdlg, IDC_TRDLIST);
									if(g_tetrFlg)
										UpdateTrdList(hWndTrdList, info);
									else
										DisplayTrdList(hWndTrdList, info);
									g_tetrFlg = 1;
								}
							}
						}
					}
					dev_close();
				}
				EnableWindow(hWndLoadBtn, TRUE);
	    		return TRUE;
			}
    	case IDC_PLDFILEBROWSE:
    		{					// IDC_PLDFILEBROWSE
				/*OPENFILENAME ofn;
				TCHAR szFile[MAX_PATH] = _BRDC(""); // путь и имя файла
				TCHAR szFileTitle[MAX_PATH];// имя файла без пути
				TCHAR szFilter[MAX_PATH] = _BRDC("PLD files\0*.mcs\0");
				TCHAR InitialDir[MAX_PATH];
				GetCurrentDirectory(sizeof(InitialDir)/sizeof(TCHAR), InitialDir);
				memset(&ofn, 0, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = NULL;
				ofn.lpstrFilter = szFilter;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = szFile;
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFileTitle = szFileTitle;
				ofn.nMaxFileTitle = sizeof(szFileTitle);
				ofn.lpstrTitle = _BRDC("Open PLD file");
				ofn.lpstrInitialDir = InitialDir;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				if(!GetOpenFileName(&ofn))
					return FALSE;*/
				HWND hWndPldName = GetDlgItem(hdlg, IDC_PLDFILENAME);
				BROWSEINFO bi;
				TCHAR szDir[MAX_PATH];
				LPITEMIDLIST pidl;
				LPMALLOC pMalloc;
				int nRet;
				GetCurrentDirectory(MAX_PATH, szDir);
				if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
				{
					nRet = 1;
					ZeroMemory(&bi,sizeof(bi));
					bi.hwndOwner = hdlg;
					bi.pszDisplayName = szDir;
					bi.lpszTitle = _T("Select the folder of PLD files, please!");
					bi.pidlRoot = NULL;//ConvertPathToLpItemIdList(RootPath);
					bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;//BIF_USENEWUI;//BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
					bi.lpfn = BrowseCallbackProc;//NULL;
					pidl = SHBrowseForFolder(&bi);
					if (pidl) 
					   SHGetPathFromIDList(pidl, szDir);
					else
						nRet = 0;
					pMalloc->Free(pidl); 
					pMalloc->Release();
				}
				if(nRet)
					SetCurrentDirectory(szDir);
				int file_cnt = (int)SendMessage(hWndPldName, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
				for(int i = file_cnt-1; i >=0 ; i--)
					SendMessage(hWndPldName, CB_DELETESTRING, (WPARAM)i, (LPARAM)0);
				//SetDlgItemText(hdlg, IDC_PLDFILENAME, szFile);
				//SendMessage(hWndPldName, CB_ADDSTRING, (WPARAM)0, (LPARAM)szFile);
				TCHAR pldname[MAX_STRING_LEN];
				BasemodName(g_DevID, pldname);
				_tcslwr(pldname); // Convert a string to lowercase
				_tcscat(pldname, _T("_*"));
				WIN32_FIND_DATA FindFileData;
				HANDLE hFind = FindFirstFile(pldname, &FindFileData);
			    if(hFind != INVALID_HANDLE_VALUE) 
				{
					SendMessage(hWndPldName, CB_ADDSTRING, (WPARAM)0, (LPARAM)FindFileData.cFileName);
				    while (FindNextFile(hFind, &FindFileData) != 0) 
				    {
						SendMessage(hWndPldName, CB_ADDSTRING, (WPARAM)0, (LPARAM)FindFileData.cFileName);
					}
					FindClose(hFind);
				}
				SendMessage(hWndPldName, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
				//SetDlgItemText(hdlg, IDC_PLDFILENAME, szFile);
	    		return TRUE;
			}
    	case IDB_ERASEICR:
    		{					// IDB_ERASEICR
				int ret = MessageBox(NULL, _BRDC("Base Module ICR will be ERASED!!!\n Are you sure?"), _BRDC("AMBPEX Config"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING); 
				if(ret == IDYES)
				{
					HWND hWndEraBtn = GetDlgItem(hdlg, IDB_ERASEICR);
					EnableWindow(hWndEraBtn, FALSE);
					int err = dev_open(g_dev);
					if(err < 0)
					{
						err = dev_openPlx(g_dev);
						if(err < 0)
							MessageBox(NULL, _BRDC("Device open error!!!"), _BRDC("AMBPEX Config"), MB_OK); 
					}
					if(err >= 0)
					{
						TCHAR buf[MAX_STRING_LEN];
						UCHAR eeprom_data[256];
						for(int i = 0; i < 256; i++)
							eeprom_data[i] = 0xFF;
						WriteNvRAM(eeprom_data, 256, 0x80);
						ReadNvRAM(eeprom_data, 256, 0x80);
						USHORT btag = *(USHORT*)eeprom_data;
						TCHAR pldDescrBuf[MAX_STRING_LEN];
						if(btag == 0x4953)
						{
							TCHAR typestr[MAX_STRING_LEN];
							ULONG ser_num = *(ULONG*)(eeprom_data+6);
							ULONG base_type = *(USHORT*)(eeprom_data+10);
							ULONG base_ver = *(eeprom_data+12);
							BasemodName(base_type, typestr);
							_stprintf(buf, _T("%s (0x%04X) : s/n = %d, version = %d.%d"), typestr, base_type, ser_num, base_ver>>4, base_ver&0xF);
							SetDlgItemText(hdlg, IDC_BASESN, buf);
							GetPldDescription(pldDescrBuf, eeprom_data, 256);
							SetDlgItemText(hdlg, IDC_PLDDESCR, pldDescrBuf);
							g_basemFlg = 1;
						}
						else
						{
							SetDlgItemText(hdlg, IDC_BASESN, _T("Base ICR error!!!"));
							_stprintf(pldDescrBuf, _T("Base ICR error!!!"));
							g_basemFlg = 0;
						}
						dev_close();
					}
					EnableWindow(hWndEraBtn, TRUE);
				}
    			return TRUE;
			}
    	case IDB_UPD:
    		{					// IDB_UPD
				HWND hWndUpdBtn = GetDlgItem(hdlg, IDB_UPD);
				EnableWindow(hWndUpdBtn, FALSE);
				int err = dev_open(g_dev);
				if(err < 0)
				{
					err = dev_openPlx(g_dev);
					if(err < 0)
						MessageBox(NULL, _BRDC("Device open error!!!"), _BRDC("AMBPEX Config"), MB_OK); 
				}
				if(err >= 0)
				{
					TRD_INFO info[MAX_TETRNUM];
					TetradList(info);
					HWND hWndTrdList = GetDlgItem(hdlg, IDC_TRDLIST);
					if(g_tetrFlg)
						UpdateTrdList(hWndTrdList, info);
					else
						DisplayTrdList(hWndTrdList, info);
					g_tetrFlg = 1;
					dev_close();
				}
				EnableWindow(hWndUpdBtn, TRUE);
	    		return TRUE;
			}
    	case IDB_GRST:
    		{					// IDB_GRST
				HWND hWndGenRstBtn = GetDlgItem(hdlg, IDB_GRST);
				EnableWindow(hWndGenRstBtn, FALSE);
				int err = dev_open(g_dev);
				if(err < 0)
				{
					err = dev_openPlx(g_dev);
					if(err < 0)
						MessageBox(NULL, _BRDC("Device open error!!!"), _BRDC("AMBPEX Config"), MB_OK); 
				}
				if(err >= 0)
				{
					//ULONG value = 1;
					//ULONG status = WriteRegData(0, 0, 0, value);
					//value = 0;
					//status = WriteRegData(0, 0, 0, value);
					//PLD_INFO pld_info;
					//err = AdmPldWorkAndCheck(&pld_info);
					ULONG value = 0;
					ULONG status = 0;
					for(int iTetr = 0; iTetr < 16; iTetr++)
					{
						value = 1;
						status = WriteRegData(0, iTetr, 0, value);
						for(int iReg = 1; iReg < 32; iReg++)
						{
							value = 0;
							status = WriteRegData(0, iTetr, iReg, value);
						}
					}
					for(int iTetr = 0; iTetr < 16; iTetr++)
					{
						value = 0;
						status = WriteRegData(0, iTetr, 0, value);
					}
					dev_close();
				}
				EnableWindow(hWndGenRstBtn, TRUE);
	    		return TRUE;
			}
    	case IDB_TEST:
    		{					// IDB_MOREINFO 
				HWND hWndTestBtn = GetDlgItem(hdlg, IDB_TEST);
				EnableWindow(hWndTestBtn, FALSE);
				//if(dev_open(iDev) < 0)
				int err = dev_open(g_dev);
				if(err < 0)
				{
					err = dev_openPlx(g_dev);
					if(err < 0)
						MessageBox(NULL, _BRDC("Device open error!!!"), _BRDC("AMBPEX Config"), MB_OK); 
					else
					{
						ULONG valueVER = 0;
						ULONG status = ReadRegData(0, 0, ADM2IFnr_VER, valueVER);
						ULONG valueMOD = 0;
						status = ReadRegData(0, 0, ADM2IFnr_IDMOD, valueMOD);
						if((valueMOD == 8 && valueVER >= 0x104) || (valueMOD == 17))
						{
							//MessageBox(NULL, L"Test MAIN tetrad!!!", L"ISDCLASS", MB_OK); 
							status = DmaChannelTest(0, 0);
						}
						else
						{
							if(valueMOD == 18)
								status = DmaChannelTest(0, 1);
							else
							{
								TRD_INFO info[MAX_TETRNUM];
								TetradList(info);
								int iTetr = 0;
								for(iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
									if(info[iTetr].id == 0x2D)
										break;
								if(iTetr != MAX_TETRNUM)
								{
									int tetr_out = iTetr;
									for(iTetr = 1; iTetr < MAX_TETRNUM; iTetr++)
										if(info[iTetr].tres & 0x10)
											break;
									if(iTetr != MAX_TETRNUM)
										BrdTest(iTetr, tetr_out);
								}
								else
									MessageBox(NULL, _BRDC("TEST tetrada (test128_out) is NOT present !!!"), _BRDC("AMBPEX Config"), MB_OK); 

								//MessageBox(NULL, _BRDC("MAIN tetrad can't be tested!!!\n No that tetrad version or modification.\nUpdate that PLD!!!"),
								//				_BRDC("AMBPEX Config"), MB_OK); 
								//wprintf(L"MAIN tetrad can't testing: version = %d.%d\n", valueVER>>8, valueVER&0xff);
							}
						}

						dev_close();
					}
				}
				else
				{
					char ver_buf[MAX_STRING_LEN];
					ULONG status = GetVersion(ver_buf);
					if(!status)
					{
						ULONG ver_major, ver_minor;
						sscanf_s(ver_buf, "wambpex ver. %i.%i", &ver_major, &ver_minor);
#ifdef _WIN64
						if(ver_major == 1 && ver_minor < 45)
							MessageBox(NULL, _BRDC("Driver version is too old !!!\nUpdate it"), _BRDC("AMBPEX Config"), MB_OK); 
						else
							if(ver_major == 1 && ver_minor < 64)
							{
								MessageBox(NULL, 
								_BRDC("Driver (version < 1.64) consists errors:\nBug#140 & Bug#1196 - errors by numbers of DMA descriptors multiple 63.\nUpdate that driver!!!"),
								_BRDC("AMBPEX Config"), MB_OK); 
							}
#else
						if(ver_major < 2)
							MessageBox(NULL, _BRDC("Driver version is too old !!!\nUpdate it"), _BRDC("AMBPEX Config"), MB_OK); 
						else
							if(ver_minor < 41)
							{
								MessageBox(NULL, 
								_BRDC("Driver (version < 2.41) consists errors:\nBug#140 & Bug#1196 - errors by numbers of DMA descriptors multiple 63.\nUpdate that driver!!!"),
								_BRDC("AMBPEX Config"), MB_OK); 
							}
#endif
					}
					//MessageBox(NULL, L"Device open success!!!", L"ISDCLASS", MB_OK); 
					ULONG valueVER = 0;
					status = ReadRegData(0, 0, ADM2IFnr_VER, valueVER);
					ULONG valueMOD = 0;
					status = ReadRegData(0, 0, ADM2IFnr_IDMOD, valueMOD);
					PCIEC_INFO pcic_info;
					status = HostReg(&pcic_info);
					if(pcic_info.coreid == 0x22 && pcic_info.pldver <= 0x104)
					{
						MessageBox(NULL, _BRDC("PLD consists error:\nBug#1455 - FM-module PRESENT is not correct.\nUpdate that PLD!!!"),
										_BRDC("AMBPEX Config"), MB_OK); 
					}
					if((pcic_info.coreid == 0x12 && pcic_info.pldver < 0x101) || (pcic_info.coreid == 0x11 && pcic_info.pldver < 0x102))
					{
						MessageBox(NULL, _BRDC("PLD consists error:\nBug#466 - FMC Power on is not correct.\nUpdate that PLD!!!"),
										_BRDC("AMBPEX Config"), MB_OK); 
					}
					if(pcic_info.dmaver < 0x102)
					{
						MessageBox(NULL, _BRDC("PLD (DMA channel version < 1.2) consists error:\nBug#182 - error by 64-bit DMA addressing (above 4GB memory).\nUpdate that PLD!!!"),
										_BRDC("AMBPEX Config"), MB_OK); 
					}
					else
					{
						if(pcic_info.dmaver < 0x103)
						{
							MessageBox(NULL, _BRDC("PLD (DMA channel version < 1.3) consists error:\nBug#147 - there is a chance the DMA channel is stopped.\nUpdate that PLD!!!"),
											_BRDC("AMBPEX Config"), MB_OK); 
						}
						if(pcic_info.errcrc != (ULONG)-1 && pcic_info.errcrc)
						{
							TCHAR buf[MAX_STRING_LEN];
							_stprintf(buf, _T("DMA descriptor CRC error counter = %d !!!"), pcic_info.errcrc);
							MessageBox(NULL, buf, _BRDC("AMBPEX Config"), MB_OK); 
						}
						if(pcic_info.errcto != (ULONG)-1 && pcic_info.errcto)
						{
							TCHAR buf[MAX_STRING_LEN];
							_stprintf(buf, _T("DMA Completion timeout error counter = %d !!!"), pcic_info.errcto);
							MessageBox(NULL, buf, _BRDC("AMBPEX Config"), MB_OK); 
						}

						//if(valueVER >= 0x104 && pcic_info.dmaver >= 0x102)
						if((valueMOD == 8 && valueVER >= 0x104) || (valueMOD == 17) || (valueMOD == 19))
						{
							//MessageBox(NULL, L"Test MAIN tetrad!!!", L"ISDCLASS", MB_OK); 
							status = DmaChannelTest(0, 0);
						}
						else
							if(valueMOD == 18 || valueMOD == 20)
								status = DmaChannelTest(0, 1);
							else
							{
								MessageBox(NULL, _BRDC("MAIN tetrad can't be tested!!!\n No that tetrad version or modification.\nUpdate that PLD!!!"),
												_BRDC("AMBPEX Config"), MB_OK); 
								//wprintf(L"MAIN tetrad can't testing: version = %d.%d\n", valueVER>>8, valueVER&0xff);
							}
					}
					dev_close();
				}
				EnableWindow(hWndTestBtn, TRUE);

    		return TRUE;
    		}					// IDB_MOREINFO
    	case IDB_POWER:
    		{
				HWND hWndPowerBtn = GetDlgItem(hdlg, IDB_POWER);
				EnableWindow(hWndPowerBtn, FALSE);
				//TCHAR tCaption[32];
				//GetWindowText(hWndPowerBtn, tCaption, 32);
				//lstrcpy(tCaption, "qwwerty");
				//GetDlgItemText(hdlg, IDB_POWER, tCaption, 32);

				if(dev_open(g_dev) < 0)
				{
					MessageBox(NULL, _BRDC("Device open error!!!"), _BRDC("AMBPEX Config"), MB_OK); 
				}
				else
				{
					//MessageBox(NULL, L"Device open success!!!", L"ISDCLASS", MB_OK); 
					FMC_POWER pow_stat;
					GetPower(&pow_stat);
					if(pow_stat.onOff)
					{
						pow_stat.onOff = 0;
						PowerOnOff(&pow_stat, 1);
						//SetDlgItemText(hdlg, IDB_POWER, _T("On"));
						//SetDlgItemText(hdlg, IDC_POWSTAT, _T("Power is Off"));
					}
					else
					{
						pow_stat.onOff = 1;
						PowerOnOff(&pow_stat, 1);
						//SetDlgItemText(hdlg, IDB_POWER, _T("Off"));
						//SetDlgItemText(hdlg, IDC_POWSTAT, _T("Power is On"));
					}
					GetPower(&pow_stat);
					if(pow_stat.onOff)
					{
						SetDlgItemText(hdlg, IDB_POWER, _T("Off"));
						SetDlgItemText(hdlg, IDC_POWSTAT, _T("Power is On"));
					}
					else
					{
						SetDlgItemText(hdlg, IDB_POWER, _T("On"));
						SetDlgItemText(hdlg, IDC_POWSTAT, _T("Power is Off"));
					}
					dev_close();
				}
				EnableWindow(hWndPowerBtn, TRUE);

    		return TRUE;
    		}
		//case IDC_APPLY:
		//	if(flSave == 1)
		//	{
		//		TCHAR itemText[32];
		//		GetDlgItemText(hDlg, IDC_NAME, itemText, 32);
		//		HWND hWndLinks = GetDlgItem(hDlg, IDC_LINKS);
		//		TCHAR itemTextOld[32];
		//		SendMessage(hWndLinks, LB_GETTEXT, (WPARAM)curLink, (LPARAM)itemTextOld);
		//		int num = (int)SendMessage(hWndLinks, LB_DELETESTRING, (WPARAM)curLink, (LPARAM)0);
		//		SendMessage(hWndLinks, LB_INSERTSTRING, (WPARAM)curLink, (LPARAM)itemText);
		//		SendMessage(hWndLinks, LB_SETCURSEL, (WPARAM)curLink, (LPARAM)0);
		//		_tcscat(itemText, _T(".ini"));
		//		_tcscat(itemTextOld, _T(".ini"));
		//		SetIniFileParam(itemTextOld, itemText);
		//		flSave = 0;
		//		ApplyDisable(hDlg, flSave);
		//	}
		//	break;
		//case IDC_ABOUT:
		//	DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hDlg, (DLGPROC)About);
		//	break;
		//case IDHELP:
		//	{
		//	HINSTANCE ret = ShellExecute(hDlg, _T("open"), sHelpFileName, NULL, NULL, SW_SHOWNORMAL);
		//	if((int)ret == ERROR_FILE_NOT_FOUND)
		//		DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hDlg, (DLGPROC)About);
		//	}
		//	break;
		case IDOK:
		case IDCANCEL:
			{
//			EndDialog(hDlg, LOWORD(wParam));
			DestroyWindow(g_hWnd);
			return TRUE;
			}
		break;
		}
	}
	return FALSE;
}

