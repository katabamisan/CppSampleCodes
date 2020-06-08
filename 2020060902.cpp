// STL&ATLでレジストリからShellExコマンドの一覧を非GUID形式とGUID形式で分けて取得する。

#include <algorithm>
#include <iostream>
#include <string>
#include <map>
#include <unordered_set>
#include <vector>
#include <regex>

#define STRICT
#define NOMINMAX // STLとの競合回避
#include <Windows.h>

#include <atlbase.h>

using namespace std;
using namespace ATL;

vector<wstring> GetSubKeyNames(HKEY hKey)
{
	vector<wstring> keyNames;
	wchar_t keyName[MAX_PATH];

	for (DWORD i = 0; i < numeric_limits<DWORD>::max(); i++)
	{
		DWORD keyNameSize = MAX_PATH;
		if (RegEnumKeyExW(hKey, i, data(keyName),
			&keyNameSize, nullptr, nullptr, 0, nullptr) != ERROR_SUCCESS)
		{
			break;
		}
		keyNames.push_back(keyName);
	}

	return keyNames;
}

// すべての要素を判別式の真偽でどちらかのイテレーターにコピーします。
template<typename InIt, typename TrueOutIt, typename FalseOutIt, typename Pred>
void copy_if_else(InIt first, InIt last, TrueOutIt trueOut, FalseOutIt falseOut, Pred pred)
{
	for (InIt i = first; i != last; i++)
	{
		if (pred(*i))
			*trueOut++ = *i;
		else
			*falseOut++ = *i;
	}
}

// ShellExキーを持つ<FileExt|ProgID>キーの名前とShellExのマップを作成する。
map<wstring, vector<wstring>> CreateFileExtOrProgIDForShellExCommandsMap(HKEY hKeyClassesRoot)
{
	map<wstring, vector<wstring>> progIdAndShellExMap;
	for (const auto& keyName : GetSubKeyNames(hKeyClassesRoot))
	{
		CRegKey shellExKey;
		LSTATUS ls = shellExKey.Open(hKeyClassesRoot, data(keyName + L"\\ShellEx"),
			KEY_READ | KEY_WOW64_64KEY);
		if (ls != ERROR_SUCCESS)
		{
			continue;
		}
		progIdAndShellExMap[keyName] = move(GetSubKeyNames(shellExKey));
	}
	return move(progIdAndShellExMap);
}

template<typename T>
basic_regex<T> make_guid_regex();

template<>
wregex make_guid_regex<wchar_t>()
{
	return wregex(
		LR"(\{[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\})",
		regex_constants::icase);
}

int main()
{
	CRegKey classesRootKey;
	classesRootKey.Open(HKEY_CLASSES_ROOT, nullptr,
		KEY_READ | KEY_WOW64_64KEY);

	map<wstring, vector<wstring>> progIdAndShellExMap
		= CreateFileExtOrProgIDForShellExCommandsMap(classesRootKey);

	unordered_set<wstring> shellExCommandSet;
	for (const auto& [name, entries] : progIdAndShellExMap)
	{
		shellExCommandSet.insert(cbegin(entries), cend(entries));
	}

	wregex reGuid(make_guid_regex<wchar_t>());
	vector<wstring> shellExCommandsAsGUID, shellExCommandsAsNoGUID;
	copy_if_else(
		cbegin(shellExCommandSet), cend(shellExCommandSet),
		back_inserter(shellExCommandsAsGUID), back_inserter(shellExCommandsAsNoGUID),
		[&reGuid](const auto& command) {return regex_match(command, reGuid); });

	wcout << "非GUID形式のShellExコマンド:" << endl;
	copy(cbegin(shellExCommandsAsNoGUID), cend(shellExCommandsAsNoGUID),
		ostream_iterator<wstring, wchar_t>(wcout, L", "));
	wcout << endl;
	wcout << "GUID形式のShellExコマンド:" << endl;
	copy(cbegin(shellExCommandsAsGUID), cend(shellExCommandsAsGUID),
		ostream_iterator<wstring, wchar_t>(wcout, L", "));
	wcout << endl;

	return 0;
}
