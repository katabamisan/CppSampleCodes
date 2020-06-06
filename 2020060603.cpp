/*
  DLLのバージョン情報のVS_FIXEDFILEINFOを取得する
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

	auto verInfoBuffer = StlGetFileVerInfo(path);
	if (verInfoBuffer.empty())
	{
		OutputDebugStringW(L"バージョン情報を取得できませんでした。");
		return GetLastError();
	}

	UINT fixedFileInfoSize;
	VS_FIXEDFILEINFO* pFixedFileInfo;
	if (!VerQueryValueW(verInfoBuffer.data(), L"\\", (LPVOID*)&pFixedFileInfo, &fixedFileInfoSize))
	{
		OutputDebugStringW(L"VS_FIXEDFILEINFOを取得できませんでした。");
		return GetLastError();
	}

	wstringstream wss;
	wss << path << endl;
	wss << L"dwFileVersionLS:    0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileVersionLS << endl;
	wss << L"dwFileVersionMS:    0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileVersionMS << endl;
	wss << L"dwProductVersionLS: 0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwProductVersionLS << endl;
	wss << L"dwProductVersionMS: 0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwProductVersionMS << endl;
	wss << L"dwFileDateLS:       0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileDateLS << endl;
	wss << L"dwFileDateMS:       0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileDateMS << endl;
	wss << L"dwFileOS:           0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileOS << endl;
	wss << L"dwFileType:         0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileType << endl;
	wss << L"dwFileSubtype:      0x" << hex << setfill(L'0') << setw(8) << pFixedFileInfo->dwFileSubtype << endl;
	OutputDebugStringW(wss.str().c_str());

	return 0;
}
