#include "Demo.h"
#include "GameApp.h"
#include <windows.h>
#include <string>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;				// current instance
WCHAR szTitle[MAX_LOADSTRING];			// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];		// the main window class name

ATOM			    MyRegisterClass(HINSTANCE hInstance);
BOOL			    InitInstance(HINSTANCE, int, LPWSTR);
BOOL			    InitWindows(HINSTANCE, int, HWND*);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
std::string		    wstr_to_str(const std::wstring& wstr, UINT CodePage = CP_OEMCP);
LPSTR*			    CommandLineToArgvA(LPCWSTR lpCmdLine, __out int* pNumArgs);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
		_In_opt_ HINSTANCE hPrevInstance,
		_In_ LPWSTR lpCmdLine,
		_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MYICON, szWindowClass, MAX_LOADSTRING);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow, lpCmdLine))
	{
		return FALSE;	
	}

	int result = GameApp::getInstance()->Run();

	GameApp::getInstance()->Clear();

	GameApp::destroyInstance();

	return result;
}

std::string wstr_to_str(const std::wstring& wstr, UINT CodePage)
{
	std::string str;
	int buffer_size = 0;
	char* pchar_str = NULL;
	buffer_size = WideCharToMultiByte(CodePage, NULL, wstr.c_str(), -1, 0, 0, NULL, FALSE);
	pchar_str = new char[buffer_size + 1];
	if (pchar_str == NULL)
	{
		return str;
	}

	memset(pchar_str, 0, buffer_size + 1);
	if (WideCharToMultiByte(CodePage, NULL, wstr.c_str(), -1, pchar_str, buffer_size, NULL, FALSE))
	{
		str = pchar_str;
	}

	delete pchar_str;
	return str;
}

LPSTR* CommandLineToArgvA(LPCWSTR lpCmdLine, __out int* pNumArgs)
{
	LPWSTR* szArgList;
	int argCount;

	if (lpCmdLine)
	{
		*pNumArgs = 0;
		return NULL;	
	}

	szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
	if (szArgList == NULL)
	{
		*pNumArgs = 0;
		return NULL;
	}

	char** szArgListA = NULL;

	szArgListA = (char**)LocalLock(LocalAlloc(LMEM_FIXED, sizeof(char*)*argCount));
	for (int i = 0; i < argCount; i++)
	{
		std::string str = wstr_to_str(szArgList[i]);
		szArgListA[i] = (char*)LocalLock(LocalAlloc(LMEM_FIXED, str.length() + 1));
		strcpy_s(szArgListA[i], str.length() + 1, str.c_str());
	}

	LocalFree(szArgList);

	*pNumArgs = argCount;

	return szArgListA;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize		    = sizeof(WNDCLASSEXW);
	wcex.style		    = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon		    = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CCRENDERER));
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= szWindowClass;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
	
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, LPWSTR lpCmdLine)
{
	hInst = hInstance;

	HWND hWnd;
	if (!InitWindows(hInstance, nCmdShow, &hWnd))
	{
		return FALSE;
	}

	LPSTR* argv;
	int argc;

	argv = CommandLineToArgvA(lpCmdLine, &argc);

	if (FAILED(!GameApp::getInstance()->Init(hWnd, argc, argv)))
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

BOOL InitWindows(HINSTANCE hInstance, int nCmdShow, HWND* hWnd)
{
	RECT wr = {0, 0, 1920, 1080};

	LONG lStyle = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);

	AdjustWindowRect(&wr, lStyle, FALSE);

	*hWnd = CreateWindowW(szWindowClass, szTitle, lStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, hInstance, nullptr);

	if (!(*hWnd))
	{
		return FALSE;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GameApp::getInstance()->OnEvent(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
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
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_SIZE:
		{
			GameApp::getInstance()->OnSize(HIWORD(lParam), LOWORD(lParam));
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


