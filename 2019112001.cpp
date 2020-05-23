//
// Windowsファイルシステムオブジェクトからサムネイルを取得して描画する
//

#pragma comment(lib, "msimg32.lib")

#define STRICT
#include <Windows.h>
#include <WindowsX.h>
#include <ShlObj.h>
#include <thumbcache.h>

ATOM RegisterMyWindowClass();
LRESULT CALLBACK MyWindowWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
// ウィンドウメッセージクラッカーハンドラ
BOOL MyWindow_OnCreate(HWND hWnd, LPCREATESTRUCT pcs);
void MyWindow_OnClose(HWND hWnd);
void MyWindow_OnPaint(HWND hWnd);

const WCHAR szMyWindowClassName[] = L"MyWindowClass";
const WCHAR szMyWindowTitle[] = L"Sample";

// TODO:サムネイルを取得するファイルシステムオブジェクトのパスを指定します。
const WCHAR imagePath[] = L"C:\\Users";
// TODO:取得するイメージの寸法を指定します。
const int imageSize = 150;

ISharedBitmap* pBitmap = nullptr;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	// COMの初期化
	auto hr = CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		// TODO:COM初期化失敗時の処理
		return -1;
	}

	// ウィンドウのクラス登録の作成
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
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// COMの解放
	CoUninitialize();

	return hr;
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
	return 0;
}

BOOL MyWindow_OnCreate(HWND hWnd, LPCREATESTRUCT pcs)
{
	// ファイルシステムオブジェクトのサムネイルのISharedBitmapを取得
	if (IThumbnailCache* pThumbnailCache = nullptr;
		SUCCEEDED(CoCreateInstance(CLSID_LocalThumbnailCache, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pThumbnailCache))))
	{
		if (IShellItem* pItem = nullptr;
			SUCCEEDED(SHCreateItemFromParsingName(imagePath, nullptr,
				IID_PPV_ARGS(&pItem))))
		{
			ISharedBitmap *pThumb = nullptr;
			WTS_CACHEFLAGS flags;
			WTS_THUMBNAILID thumbId;

			pThumbnailCache->GetThumbnail(
				pItem, 100, WTS_FLAGS::WTS_NONE,
				&pBitmap, &flags, &thumbId);
		}
		pThumbnailCache->Release();
	}

	return TRUE;
}

void MyWindow_OnClose(HWND hWnd)
{
	PostQuitMessage(0);
}

void MyWindow_OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	auto hdc = BeginPaint(hWnd, &ps);
	auto hdcBitmap = CreateCompatibleDC(hdc);

	// ARGB形式ビットマップの描画
	HBITMAP hbmp = nullptr;
	pBitmap->GetSharedBitmap(&hbmp);
	BITMAP bmp;
	GetObject(hbmp, sizeof(bmp), &bmp);
	SelectObject(hdcBitmap, hbmp);
	BLENDFUNCTION blendFunc;
	blendFunc.BlendOp = AC_SRC_OVER;
	blendFunc.BlendFlags = 0;
	blendFunc.SourceConstantAlpha = 0xff;
	blendFunc.AlphaFormat = AC_SRC_ALPHA;
	AlphaBlend(hdc, 0, 0, imageSize, imageSize,
		hdcBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, blendFunc);

	DeleteDC(hdcBitmap);
	EndPaint(hWnd, &ps);
}
