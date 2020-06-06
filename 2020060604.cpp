/*
  DLLのバージョン情報のTranslation（ロケールIDとコードページ）を取得する
  作成日：2020/06/06
  最終更新日：2020/06/06
  開発環境：Microsoft Visual Studio Community 2019 Version 16.7.0 Preview 2.0
 */

#pragma comment(lib, "version.lib")

 // STL
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <span>
#include <vector>

// Windows API Headers
#define STRICT
#include <Windows.h>
#include <winver.h>

// using namespaces
using namespace std;

vector<std::byte> StlGetFileVerInfo(wstring_view path)
{
	vector<std::byte> verInfo;
	if (UINT size = GetFileVersionInfoSizeExW(0, path.data(), nullptr); size != 0)
	{
		verInfo.resize(size);
		if (!GetFileVersionInfoExW(0, path.data(), 0, size, verInfo.data()))
		{
			return {};
		}
	}
	else
	{
		return {};
	}
	return verInfo;
}

vector<DWORD> StlGetFileVerInfoTranslations(span<std::byte> verInfo)
{
	DWORD* pTranslation;
	UINT translationSize;
	if (!VerQueryValueW(verInfo.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&pTranslation, &translationSize))
	{
		return {};
	}
	return vector<DWORD>(pTranslation, pTranslation + (translationSize / sizeof(DWORD)));
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	auto path = L"gdiplus.dll";

	auto verInfo = StlGetFileVerInfo(path);
	if (verInfo.empty())
	{
		OutputDebugStringW(L"バージョン情報を取得できませんでした。");
		return GetLastError();
	}

	auto translations = StlGetFileVerInfoTranslations(verInfo);
	if (translations.empty())
	{
		OutputDebugStringW(L"バージョン情報の言語情報を取得できませんでした。");
		return GetLastError();
	}

	wstringstream wss;
	wss << path << endl;
	for (auto&& translation : translations)
	{
		wss << L"0x" << hex << setfill(L'0') << setw(8) << translation << endl;
	}
	OutputDebugStringW(wss.str().c_str());

	return 0;
}
