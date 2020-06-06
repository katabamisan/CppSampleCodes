/*
  GDI+の初期化・解放とエンコーダーの列挙
  作成日：2020/06/06
  最終更新日：2020/06/06
  対応バージョン：C++20（std::span）
  開発環境：Microsoft Visual Studio Community 2019 Version 16.7.0 Preview 2.0
 */

#pragma comment(lib, "gdiplus.lib")

 // STL
#include <cstddef>
#include <span>
#include <sstream>
#include <vector>

// Windows API Headers
#define STRICT
#include <Windows.h>
#include <gdiplus.h>

// using namespaces
using namespace std;
using namespace Gdiplus;

tuple<vector<std::byte>, UINT, Status> StlGetImageEncoders()
{
	UINT numEncoders, size;
	if (auto status = GetImageEncodersSize(&numEncoders, &size); status != Status::Ok)
	{
		return {{}, 0, status};
	}

	vector<std::byte> buffer(size);
	if (auto status = GetImageEncoders(numEncoders, size, (ImageCodecInfo*)buffer.data()); status != Status::Ok)
	{
		return {{}, 0, status};
	}
	return {move(buffer), numEncoders, Status::Ok};
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	GdiplusStartupInput gdiplusStartpuInput;
	ULONG_PTR gdiplusToken;
	if (auto status = GdiplusStartup(&gdiplusToken, &gdiplusStartpuInput, nullptr); status != Status::Ok)
	{
		OutputDebugStringW(L"GDI+の初期化に失敗しました。");
		return status;
	}

	auto [encodersBuffer, numEncoders, status] = StlGetImageEncoders();
	span encoders((ImageCodecInfo*)encodersBuffer.data(), numEncoders);
	if (status != Status::Ok)
	{
		OutputDebugStringW(L"エンコーダーの取得に失敗しました。");
		return 0;
	}

	wstringstream wss;
	wss << L"CodecName, FormatDescription, FilenameExtension" << endl;
	for (auto&& encoder : encoders)
	{
		wss << encoder.CodecName << L", " << encoder.FormatDescription << L", " << encoder.FilenameExtension << endl;
	}
	OutputDebugStringW(wss.str().c_str());

	GdiplusShutdown(gdiplusToken);

	return 0;
}
