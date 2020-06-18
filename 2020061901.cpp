// すべてのスレッドから説明の設定されたスレッドのID、説明、実行ファイル名を取得する。
//
// 動作確認環境：
// Microsoft Visual Studio Community 2019 Preview Version 16.7.0 Preview 2.0
// Visual C++ 2019
// C++ 言語標準：C++20プレビュー（/std:c++latest）

#pragma comment(lib, "shlwapi.lib")

// STL ヘッダー
#include <algorithm>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <string_view>
#include <memory>
#include <vector>

// Win32 API ヘッダー
#define STRICT
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <Psapi.h>

// 名前空間の省略
using namespace std;

struct win32_handle_deleter
{
	void operator()(HANDLE handle) const noexcept
	{
		CloseHandle(handle);
	}
};

using unique_win32_handle = unique_ptr<remove_pointer_t<HANDLE>, win32_handle_deleter>;

vector<PROCESSENTRY32W> StlToolhelp32GetAllProcessEntriesW(HANDLE hSnapshot)
{
	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(pe);
	vector<PROCESSENTRY32W> entries;
	if (Process32FirstW(hSnapshot, &pe)) {
		do {
			entries.push_back(pe);
		} while (Process32NextW(hSnapshot, &pe));
	}
	return entries;
}

vector<THREADENTRY32> StlToolhelp32GetAllThreadEntries(HANDLE hSnapshot)
{
	THREADENTRY32 te;
	te.dwSize = sizeof(te);
	vector<THREADENTRY32> entries;
	if (Thread32First(hSnapshot, &te)) {
		do {
			entries.push_back(te);
		} while (Thread32Next(hSnapshot, &te));
	}
	return entries;
}

wstring StlGetThreadDescription(DWORD dwThreadId)
{
	wstring result;
	const auto threadHandle = OpenThread(GENERIC_READ, FALSE, dwThreadId);
	if (threadHandle != nullptr) {
		if (LPWSTR psz; SUCCEEDED(GetThreadDescription(threadHandle, &psz))) {
			result = psz;
			LocalFree(psz);
		}
		CloseHandle(threadHandle);
	}
	return result;
}

wstring_view StlPathStripPathW(wstring_view path)
{
	auto p = data(path);
	PathStripPathW(const_cast<LPWSTR>(p));
	return wstring_view(p);
}

int main()
{
	const auto snapshotHandle = unique_win32_handle(
		CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0));
	if (!snapshotHandle) {
		return 1;
	}

	// すべてのプロセスおよびスレッド情報の取得
	const auto processEntries = StlToolhelp32GetAllProcessEntriesW(snapshotHandle.get());
	const auto threadEntries = StlToolhelp32GetAllThreadEntries(snapshotHandle.get());

	// スレッド情報→スレッド情報と説明のタプル
	vector<tuple<THREADENTRY32, wstring>> threadInfos;
	threadInfos.reserve(threadEntries.capacity());
	transform(cbegin(threadEntries), cend(threadEntries),
		back_insert_iterator(threadInfos),
		[](const THREADENTRY32& entry) {
			return tuple(entry, StlGetThreadDescription(entry.th32ThreadID));
		});

	// 説明が空の要素を除去
	threadInfos.erase(
		remove_if(begin(threadInfos), end(threadInfos),
			[](const tuple<THREADENTRY32, wstring>& info) {
				return empty(get<wstring>(info));
			}),
		end(threadInfos));

	// スレッド情報と説明のタプル→スレッド情報と説明とプロセスのファイル名のタプル
	vector<tuple<THREADENTRY32, wstring, wstring>> threadProcessInfo;
	threadProcessInfo.reserve(threadInfos.capacity());
	transform(cbegin(threadInfos), cend(threadInfos),
		back_insert_iterator(threadProcessInfo),
		[&processEntries](const tuple<THREADENTRY32, wstring>& info) {
			auto i = find_if(cbegin(processEntries), cend(processEntries),
				[&info](const PROCESSENTRY32& entry) {
					return get<0>(info).th32OwnerProcessID == entry.th32ProcessID;
				});
			wstring path;
			if (i != processEntries.cend()) {
				path = StlPathStripPathW((*i).szExeFile);
			}
			return tuple(get<0>(info), get<1>(info), move(path));
		});

	// 出力
	for (const auto& [info, desc, filename] : threadProcessInfo) {
		wcout << hex << setw(8) << setfill(L'0') << info.th32ThreadID
			<< L": " << desc << " (" << filename << ")" << endl;
	}

	return 0;
}
