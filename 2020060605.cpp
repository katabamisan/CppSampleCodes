/*
  DLLのバージョン情報の基本的な文字列値を取得する
  作成日：2020/06/06
  最終更新日：2020/06/06
  開発環境：Microsoft Visual Studio Community 2019 Version 16.7.0 Preview 1.0
 */

#pragma comment(lib, "version.lib")

 // STL
#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <span>
#include <map>
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

vector<DWORD> StlGetFileVerInfoLangAndCodePages(span<std::byte> verInfo)
{
	vector translations(StlGetFileVerInfoTranslations(verInfo));
	transform(translations.begin(), translations.end(), translations.begin(),
		[](DWORD translation) {return MAKELONG(HIWORD(translation), LOWORD(translation)); });
	return translations;
}

map<DWORD, wstring> StlGetFileVerInfoStringFileInfoValues(span<std::byte> verInfo, wstring_view valueName)
{
	map<DWORD, wstring> values;
	wstring subblock(sizeof(L"\\StringFileInfo\\00000000\\") + valueName.size() + sizeof(L"\0"), L'\0');
	for (DWORD translation : StlGetFileVerInfoLangAndCodePages(verInfo))
	{
		swprintf(subblock.data(), subblock.size(),
			(L"\\StringFileInfo\\%08x\\"s + valueName.data()).data(),
			translation);
		UINT valueSize;
		LPVOID valuePtr;
		if (VerQueryValueW(verInfo.data(), subblock.data(), &valuePtr, &valueSize))
		{
			values[translation] = move(wstring((LPCWSTR)valuePtr));
		}
	}
	return values;
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
	const auto valueNames = {
			L"Comments", L"CompanyName", L"FileDescription",
			L"FileVersion", L"InternalName", L"LegalCopyright",
			L"LegalTrademarks", L"OriginalFilename", L"ProductName",
			L"ProductVersion", L"PrivateBuild", L"SpecialBuild",
		};
	for (auto&& valueName : valueNames)
	{
		wss << hex << setfill(L'0') << setw(8) << valueName << endl;
		for (auto&& [key, value] : StlGetFileVerInfoStringFileInfoValues(verInfo, valueName))
		{
			wss << L"  " << key << L": " << value << endl;
		}
	}
	OutputDebugStringW(wss.str().c_str());

	return 0;
}
