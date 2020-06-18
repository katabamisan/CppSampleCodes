// クラスの登録されたレジストリキーの名前を検索してCSVに書き出す。
//
// 動作確認環境：
// Microsoft Visual Studio Community 2019 Preview Version 16.7.0 Preview 2.0
// Visual C++ 2019
// C++ 言語標準：C++20プレビュー（/std:c++latest）

#include <string>
#include <vector>
#include <fstream>
#include <span>

#define STRICT
#include <Windows.h>

using namespace std;
using namespace std::string_view_literals;

struct RegKeyInfo
{
	wstring Path;
	wstring KeyName;
	wstring ClassName;
};

void FindRegKeysWithClassWorker(
	HKEY hKey,
	wstring path,
	vector<RegKeyInfo>& infos,
	wstring& name,
	wstring& className)
{
	DWORD numSubKeys;
	DWORD maxSubKeyLen;
	DWORD maxClassLen;
	RegQueryInfoKeyW(hKey, nullptr, nullptr, nullptr, &numSubKeys,
		&maxSubKeyLen, &maxClassLen, nullptr, nullptr, nullptr, nullptr, nullptr);

	if (name.capacity() < static_cast<size_t>(maxSubKeyLen) + 1) {
		name.resize(static_cast<size_t>(maxSubKeyLen) + 1);
	}
	if (className.capacity() < static_cast<size_t>(maxClassLen) + 1) {
		className.resize(static_cast<size_t>(maxClassLen) + 1);
	}
	for (DWORD i = 0; i < numSubKeys; i++) {
		auto cchName = maxSubKeyLen + 1;
		auto cchClassName = maxClassLen + 1;
		const auto ret = RegEnumKeyExW(hKey, i, name.data(),
			&cchName, nullptr, className.data(), &cchClassName, nullptr);
		if (ret == ERROR_SUCCESS) {
			if (cchClassName != 0) {
				infos.push_back({path.data(), name.data(), className.data()});
			}
			HKEY hSubKey;
			if (RegOpenKeyExW(hKey, name.data(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
				FindRegKeysWithClassWorker(hSubKey, path + L"\\" + name, infos, name, className);
				RegCloseKey(hSubKey);
			}
		}
	}
}

vector<RegKeyInfo> FindRegKeysWithClass(
	HKEY hKey,
	wstring_view path_prefix)
{
	vector<RegKeyInfo> infos;
	wstring name;
	wstring className;

	FindRegKeysWithClassWorker(hKey, data(path_prefix), infos, name, className);

	return infos;
}

template <typename _OutStream, typename _ContTy>
void join_dquoted(_OutStream& stream, const _ContTy& container, wstring_view sep)
{
	for (size_t i = 0; const auto & item : container) {
		stream << L'\"' << item << L'\"';
		if (++i != size(headers)) {
			stream << sep;
		} else {
			i = 0;
		}
	}
}

void write_csv(
	wstring_view filename,
	const vector<RegKeyInfo>& infos,
	const wstring_view linesep = L"\r\n")
{
	const auto& headers = {L"Path", L"KeyName", L"ClassName"};
	wofstream of(data(filename), ios_base::out);
	join_dquoted(of, headers, L", ");
	of << data(linesep);
	for (const auto& info : infos) {
		of << data(info.Path) << L"\",\"" << data(info.KeyName) << L"\",\""
			<< data(info.ClassName) << L"\"" << data(linesep);
	}
}

int main()
{
	write_csv(L"HKEY_CLASSES_ROOT.csv", FindRegKeysWithClass(HKEY_CLASSES_ROOT, L"HKEY_CLASSES_ROOT"));
	write_csv(L"HKEY_LOCAL_MACHINE.csv", FindRegKeysWithClass(HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE"));
	write_csv(L"HKEY_CURRENT_USER.csv", FindRegKeysWithClass(HKEY_CURRENT_USER, L"HKEY_CURRENT_USER"));

	return 0;
}
