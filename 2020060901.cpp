// STL&ATLでレジストリからShellExのコマンド別に拡張子とProgIDを列挙する。

#include <algorithm>
#include <iostream>
#include <string>
#include <map>
#include <unordered_set>
#include <vector>

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

// ShellExキーを持つ<FileExt|ProgID>キーの名前とShellExのマップを作成する。
map<wstring, vector<wstring>> CreateFileExtOrProgIDToShellExCommandsMap(HKEY hKeyClassesRoot)
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

int main()
{
	CRegKey classesRootKey;
	classesRootKey.Open(HKEY_CLASSES_ROOT, nullptr,
		KEY_READ | KEY_WOW64_64KEY);

	map<wstring, vector<wstring>> progIdAndShellExMap
		= CreateFileExtOrProgIDToShellExCommandsMap(classesRootKey);

	unordered_set<wstring> shellExCommandSet;
	for (const auto& [name, entries] : progIdAndShellExMap)
	{
		shellExCommandSet.insert(cbegin(entries), cend(entries));
	}

	// ShellExコマンド→FileExt|ProgID
	map<wstring, vector<wstring>> shellExCommandToProgIDsMap;
	for (const auto& command : shellExCommandSet)
	{
		vector<wstring> names;
		for (const auto& [name, entries] : progIdAndShellExMap)
		{
			if (find(entries.cbegin(), entries.cend(), command) != entries.cend())
			{
				names.push_back(name);
			}
		}
		shellExCommandToProgIDsMap[command] = move(names);
	}

	for (const auto& [command, names] : shellExCommandToProgIDsMap)
	{
		wcout << command << ": ";
		copy(cbegin(names), cend(names), ostream_iterator<wstring, wchar_t>(wcout, L", "));
		wcout << endl;
	}

	return 0;
}
