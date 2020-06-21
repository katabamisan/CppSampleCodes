// STLでIUnknown派生クラスポインタやSTRRETを扱うためのラッパークラス（com_ptr_releaser、STRRETToWStringWrapper）
//
// 動作確認環境：
// Microsoft Visual Studio Community 2019 Preview Version 16.7.0 Preview 2.0
// Visual C++ 2019
// C++ 言語標準：C++20プレビュー（/std:c++latest）

#pragma comment(lib, "shlwapi.lib")

// STL ヘッダー
#include <memory>
#include <string>

// Win32 API ヘッダー
#define STRICT
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>

// 名前空間の省略
using namespace std;

// IUnknown派生クラスのReleaseを呼び出すクラス。
template <class InterfaceT>
struct com_ptr_releaser
{
	constexpr void operator()(InterfaceT* p) const noexcept
	{
		if (p != nullptr) {
			p->Release();
		}
	}
};

// std::unique_ptrのIUnknown派生クラスに対する特殊化。
template <typename InterfaceT>
using unique_com_ptr = unique_ptr<InterfaceT, com_ptr_releaser<InterfaceT>>;

// 関数のvoid**型引数に与えて呼び出し後の値をunique_com_ptrへ代入するラッパークラス。
template <typename InterfaceT>
struct com_ptr_address {
public:
	constexpr com_ptr_address(unique_com_ptr<InterfaceT>& ptr) noexcept
		: target(ptr), p(ptr.get())
	{
	}

	~com_ptr_address() noexcept
	{
		target.reset(p);
	}

	constexpr operator InterfaceT**() noexcept
	{
		return &p;
	}

private:
	com_ptr_address() = delete;
	com_ptr_address(const com_ptr_address<InterfaceT>&) = delete;
	com_ptr_address(com_ptr_address<InterfaceT>&&) = delete;

private:
	unique_com_ptr<InterfaceT>& target;
	InterfaceT* p;
};

// 関数のSTRRET*型引数に与えて呼び出し後の値をstd::wstringへ代入するラッパークラス。
struct STRRETToWStringWrapper
{
public:
	constexpr STRRETToWStringWrapper(wstring& target, LPITEMIDLIST pidl = nullptr) noexcept
		: target(target), pidl(pidl), sr{}
	{
	}

	operator STRRET* () noexcept
	{
		return &sr;
	}

	~STRRETToWStringWrapper() noexcept
	{
		LPWSTR psz;
		if (SUCCEEDED(StrRetToStrW(&sr, pidl, &psz))) {
			target = psz;
		} else {
			target.clear();
		}
	}

private:
	wstring& target;
	LPITEMIDLIST pidl;
	STRRET sr;
	STRRETToWStringWrapper(const STRRETToWStringWrapper&) = delete;
	STRRETToWStringWrapper(STRRETToWStringWrapper&&) = delete;
};

int main()
{
	unique_com_ptr<IShellFolder> desktop;
	if (auto hr = SHGetDesktopFolder(com_ptr_address(desktop)); FAILED(hr)) {
		return hr;
	}

	wstring desktopDisplayName;
	if (auto hr = desktop->GetDisplayNameOf(nullptr, SHGDN_NORMAL, STRRETToWStringWrapper(desktopDisplayName));
		FAILED(hr)) {
		return hr;
	}

	return 0;
}
