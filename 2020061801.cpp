// すべてのスレッドから説明の設定されたスレッドのIDと説明を取得する
//
// 動作確認環境：
// Microsoft Visual Studio Community 2019 Preview Version 16.7.0 Preview 2.0
// Visual C++ 2019：00435-60000-00000-AA317
// C++ 言語標準：C++20プレビュー（/std:c++latest）
//

// STL ヘッダー
#include <algorithm>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>

// Win32 API ヘッダー
#define STRICT
#include <Windows.h>
#include <TlHelp32.h>

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

vector<DWORD> StlToolhelp32GetAllThreadIds(HANDLE hSnapshot)
{
	THREADENTRY32 te;
	te.dwSize = sizeof(te);
	vector<DWORD> ids;
	if (Thread32First(hSnapshot, &te)) {
		do {
			ids.push_back(te.th32ThreadID);
		} while (Thread32Next(hSnapshot, &te));
	}
	return ids;
}

wstring StlGetThreadDescription(DWORD dwThreadId)
{
	wstring result;
	const auto threadHandle = OpenThread(GENERIC_READ, FALSE, dwThreadId);
	if (threadHandle != NULL) {
		if (LPWSTR psz; SUCCEEDED(GetThreadDescription(threadHandle, &psz))) {
			result = psz;
			LocalFree(psz);
		}
		CloseHandle(threadHandle);
	}
	return result;
}

int main()
{
	const auto snapshotHandle = unique_win32_handle(
		CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
	if (!snapshotHandle) {
		return 1;
	}

	// すべてのスレッドIDの取得
	const auto threadIds = StlToolhelp32GetAllThreadIds(
		snapshotHandle.get());

	// スレッドID→スレッドIDと説明のタプル
	vector<tuple<DWORD, wstring>> threadInfos;
	threadInfos.reserve(threadIds.capacity());
	transform(cbegin(threadIds), cend(threadIds),
		back_insert_iterator(threadInfos),
		[](DWORD threadId) {return tuple(threadId, StlGetThreadDescription(threadId)); });

	// 説明が空の要素を除去
	threadInfos.erase(
		remove_if(begin(threadInfos), end(threadInfos),
			[](const auto& info) {return empty(get<wstring>(info)); }),
		end(threadInfos));

	// 出力
	for (const auto& [id, desc] : threadInfos) {
		wcout << hex << setw(8) << setfill(L'0') << id << L": " << desc << endl;
	}

	return 0;
}
