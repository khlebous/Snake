// snake.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "snake.h"
#include "vector"
#include <time.h>
#include <shellapi.h>
using namespace std;
#define MAX_LOADSTRING 100
#define N 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];
// the main window class name

//My global Variables
int _screenX = GetSystemMetrics(SM_CXSCREEN);
int _screenY = GetSystemMetrics(SM_CYSCREEN);
int _blockSize = 32;
int _blocksInScreenX = _screenX / _blockSize;
int _blocksInScreenY = _screenY / _blockSize;
int _myRound = 4;
int _direction = 2;
HWND _snakeWnd[100];
HWND _bonusWnd;
int _snakeSize = 3;
int _nrDiam = 0;
int _nrColor = 0;
int _points =0;

//HDC memDC;
// Forward declarations of functions included in this code module:
ATOM                MySnakeClass(HINSTANCE hInstance);
ATOM                MyBonusClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    SnakeProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    BonusProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
	MySnakeClass(hInstance);
	MyBonusClass(hInstance);
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MySnakeClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = SnakeProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(128, 0, 128));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SNAKE);
	wcex.lpszClassName = L"Snake";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}
ATOM MyBonusClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = BonusProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(255, 0, 0));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SNAKE);
	wcex.lpszClassName = L"Bonus";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

// My functions
HWND CreateBonus(HINSTANCE hInstance, int nCmdShow);
void MoveBonus();
HWND CreateBlock(int x, int y, HINSTANCE hInstance, int nCmdShow);
void GetTextInfoForKeyMsg(HWND hWnd, WPARAM wParam);
void MoveSnake();
bool CheckBonus();
void LengthenSnake();
bool Collapses(int x, int y);
void PaintBitmap();
void AddIcon(HWND hWnd);void RemoveIcon();
void ShowMenu(HWND hWnd);
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
	srand(time(NULL));

	hInst = hInstance; // Store instance handle in our global variable

	_bonusWnd = CreateBonus(hInstance, nCmdShow);

	for (int i = 0; i < _snakeSize; i++)
	{
		_snakeWnd[i] = CreateBlock(30 * _blockSize + i*_blockSize, 10 * _blockSize, hInstance, nCmdShow);
	}
	
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK SnakeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
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
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK BonusProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_KEYDOWN:
	{
		GetTextInfoForKeyMsg(hWnd, wParam);
	}
	break;
	case WM_CREATE:
		SetTimer(hWnd, 1, 300, NULL);
		SetTimer(hWnd, 2, 200, NULL);
		AddIcon(hWnd);
		break;	case WM_APP:		if (lParam == WM_RBUTTONDOWN)		{			SetForegroundWindow(hWnd);			ShowMenu(hWnd);			PostMessage(hWnd, WM_APP + 1, 0, 0);		}		break;	case WM_CLOSE:		RemoveIcon();		PostQuitMessage(0);		break;	case WM_TIMER:
	{
		if (wParam == 1)
		{
			SetFocus(hWnd);
			MoveSnake();
			if (CheckBonus())
			{
				_points++;
				_nrColor = rand() % 6;
				_nrDiam = 0;
				InvalidateRect(_bonusWnd, NULL, false);
				MoveBonus();
				LengthenSnake();
			}
		}
		if (wParam == 2)
		{
			SetFocus(hWnd);
			_nrDiam = (1 + _nrDiam) % 8;
			InvalidateRect(_bonusWnd, NULL, false);
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PaintBitmap();
		break;
	}
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

HWND CreateBlock(int x, int y, HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"Snake", nullptr, WS_POPUP | WS_SYSMENU | WS_EX_TOPMOST,
		x, y, _blockSize, _blockSize, nullptr, nullptr, hInstance, nullptr);
	::SetMenu(hWnd, nullptr);
	HRGN region1 = CreateEllipticRgn(-_myRound, -_myRound, _blockSize + _myRound, _blockSize + _myRound);
	SetWindowRgn(hWnd, region1, true);
	SetWindowPos(hWnd, HWND_TOPMOST, x, y, _blockSize, _blockSize, NULL);
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, (255 * 40) / 100, LWA_ALPHA);
	ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);
	if (!hWnd)
		return FALSE;
	return hWnd;
}

bool Collapses(int x, int y)
{
	for (int i = 0; i < _snakeSize; i++)
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[i], &lpRect);
		if (lpRect.left == x && lpRect.top == y)
			return true;
	}
	return false;
}

HWND CreateBonus(HINSTANCE hInstance, int nCmdShow)
{
	int x;
	int y;
	do {
		x = rand() % _blocksInScreenX * _blockSize;
		y = rand() % _blocksInScreenY *_blockSize;
	} while (Collapses(x, y));
	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"Bonus", nullptr, WS_POPUP | WS_SYSMENU | WS_EX_TOPMOST,
		x*_blockSize, y*_blockSize, _blockSize, _blockSize, nullptr, nullptr, hInstance, nullptr);
	::SetMenu(hWnd, nullptr);
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, RGB(51, 51, 51), 255, LWA_ALPHA | LWA_COLORKEY);
	SetWindowPos(hWnd, HWND_TOPMOST, x, y, _blockSize, _blockSize, NULL);
	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	if (!hWnd)
		return FALSE;
	return hWnd;
}

void MoveBonus()
{
	int x;
	int y;
	do {
		x = rand() % _blocksInScreenX * _blockSize;
		y = rand() % _blocksInScreenY *_blockSize;
	} while (Collapses(x, y));
	MoveWindow(_bonusWnd, x, y, _blockSize, _blockSize, true);
}

void GetTextInfoForKeyMsg(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
	case VK_RIGHT:
	{
		_direction = 0;
	}
	break;
	case VK_DOWN:
	{
		_direction = 1;
	}
	break;
	case VK_LEFT:
	{
		_direction = 2;
	}
	break;
	case VK_UP:
	{
		_direction = 3;
	}
	break;
	case VK_SPACE:
	{
		LengthenSnake();
	}
	}
}

void MoveSnake()
{
	for (int i = _snakeSize - 1; i > 0; i--)
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[i - 1], &lpRect);
		MoveWindow(_snakeWnd[i], lpRect.left, lpRect.top, _blockSize, _blockSize, false);
	}
	switch (_direction)
	{
	case 0://right
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[0], &lpRect);
		MoveWindow(_snakeWnd[0], (lpRect.left + _blockSize) % (_blocksInScreenX*_blockSize), lpRect.top, _blockSize, _blockSize, false);
	}
	break;
	case 1://down
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[0], &lpRect);
		MoveWindow(_snakeWnd[0], lpRect.left, (lpRect.top + _blockSize) % (_blocksInScreenY*_blockSize), _blockSize, _blockSize, false);
	}
	break;
	case 2://left
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[0], &lpRect);
		int x = lpRect.left - _blockSize;
		if (x < 0)
			x = _blockSize*(_blocksInScreenX - 1);
		MoveWindow(_snakeWnd[0], x, lpRect.top, _blockSize, _blockSize, false);
	}
	break;
	case 3://up
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[0], &lpRect);
		int y = lpRect.top - _blockSize;
		if (y < 0)
			y = _blockSize*(_blocksInScreenY - 1);
		MoveWindow(_snakeWnd[0], lpRect.left, y, _blockSize, _blockSize, false);
	}
	break;
	default:
		break;
	}
}

bool CheckBonus()
{
	RECT RectS;
	GetWindowRect(_snakeWnd[0], &RectS);
	RECT RectB;
	GetWindowRect(_bonusWnd, &RectB);
	return(RectS.left == RectB.left && RectS.top == RectB.top);
}

void LengthenSnake()
{
	if (_snakeSize < 100)
	{
		RECT lpRect;
		GetWindowRect(_snakeWnd[_snakeSize - 1], &lpRect);
		_snakeWnd[_snakeSize++] = CreateBlock(lpRect.left, lpRect.top, (HINSTANCE)GetWindowLong(_bonusWnd, GWL_HINSTANCE), SW_SHOW);
	}
}

void PaintBitmap()
{
	PAINTSTRUCT ps;
	HBITMAP hBitmap;
	HDC hdc = BeginPaint(_bonusWnd, &ps);
	HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDC_D));
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
	BitBlt(hdc, 0, 0, _blockSize, _blockSize, memDC, _blockSize*_nrDiam, _blockSize*_nrColor, SRCCOPY);
	SelectObject(memDC, oldBitmap);
	DeleteObject(bitmap);
	DeleteDC(memDC);
	EndPaint(_bonusWnd, &ps);
	ShowWindow(_bonusWnd, SW_SHOWDEFAULT);
}

void AddIcon(HWND hWnd)
{
	NOTIFYICONDATA ntd;
	ntd.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE;
	ntd.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SNAKE));
	ntd.hWnd = hWnd;
	ntd.uID = 1;
	ntd.uCallbackMessage = WM_APP;

	_tcscpy_s(ntd.szTip, TEXT("snake"));
	Shell_NotifyIcon(NIM_ADD, &ntd);
}

void RemoveIcon()
{
	NOTIFYICONDATA ntd;
	ntd.hWnd = _bonusWnd;
	ntd.uID = 1;
	Shell_NotifyIcon(NIM_DELETE, &ntd);
}

void ShowMenu(HWND hWnd)
{
	HMENU hMenu = CreatePopupMenu();
	const int maxBuffor = 20;
	wchar_t buffor[maxBuffor];
	_snwprintf(buffor, maxBuffor, L"points: %d", _points);
	InsertMenu(hMenu, 0, MF_DISABLED | MF_STRING | MF_BYPOSITION| MF_GRAYED, 0, buffor);
	InsertMenu(hMenu, 1, MF_STRING | MF_BYPOSITION, IDM_EXIT, L"exit   ALT+F4");
	POINT point;
	GetCursorPos(&point);
	WORD message = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
								  point.x, point.y, 0, hWnd, NULL);
	SendMessage(hWnd, WM_COMMAND, message, 0);
	DestroyMenu(hMenu);
}