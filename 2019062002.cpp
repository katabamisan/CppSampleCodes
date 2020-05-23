//
// WindowsX.hのメッセージクラッカーの基本
//

#define STRICT
#include <Windows.h>
#include <WindowsX.h>

ATOM RegisterMyWindowClass();
LRESULT CALLBACK MyWindowWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

BOOL MyWindow_OnCreate(HWND hWnd, LPCREATESTRUCT pcs);
void MyWindow_OnClose(HWND hWnd);
void MyWindow_OnPaint(HWND hWnd);

const WCHAR szMyWindowClassName[] = L"MyWindowClass";
const WCHAR szMyWindowTitle[] = L"Sample";

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	// ウィンドウクラスの登録
	auto atomWindow = RegisterMyWindowClass();
	if (atomWindow == 0)
	{
		// TODO:ウィンドウクラス登録失敗時の処理
		return -1;
	}

	auto hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		MAKEINTATOM(atomWindow),
		szMyWindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, nullptr, nullptr);
	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);
	MSG msg;
	for (int ret = GetMessage(&msg, nullptr, 0, 0);
		ret != 0;
		ret = GetMessage(&msg, nullptr, 0, 0))
	{
		if (ret == -1)
		{
			// TODO:エラー時の処理
			return -1;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

ATOM RegisterMyWindowClass()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MyWindowWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = nullptr;
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szMyWindowClassName;
	wcex.hIconSm = nullptr;
	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK MyWindowWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		HANDLE_MSG(hWnd, WM_CREATE, MyWindow_OnCreate);
		HANDLE_MSG(hWnd, WM_CLOSE, MyWindow_OnClose);
		HANDLE_MSG(hWnd, WM_PAINT, MyWindow_OnPaint);
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}

BOOL MyWindow_OnCreate(HWND hWnd, LPCREATESTRUCT pcs)
{
	// TODO:ウィンドウの作成時の処理

	return TRUE;
}

void MyWindow_OnClose(HWND hWnd)
{
	// TODO:ウィンドウの終了時の処理

	PostQuitMessage(0);
}

void MyWindow_OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	auto hdc = BeginPaint(hWnd, &ps);

	// TODO:ウィンドウの描画処理

	EndPaint(hWnd, &ps);
}
