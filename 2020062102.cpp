// STL&COMでデスクトップのファイルシステムオブジェクトの名前を列挙する
//
// 動作確認環境：
// Microsoft Visual Studio Community 2019 Preview Version 16.7.0 Preview 2.0
// Visual C++ 2019
// C++ 言語標準：C++20プレビュー（/std:c++latest）

#pragma comment(lib, "shlwapi.lib")

// STL ヘッダー
#include <iterator>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>

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

	constexpr operator InterfaceT** () noexcept
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

	constexpr operator STRRET* () noexcept
	{
		return &sr;
	}

	~STRRETToWStringWrapper() noexcept(false)
	{
		LPWSTR psz;
		if (SUCCEEDED(StrRetToStrW(&sr, pidl, &psz))) {
			target = move(psz);
		}
		else {
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

template <typename Ty>
struct CoTaskMem_allocator {
public:
	using value_type = Ty;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	using propagate_on_container_move_assigment = true_type;
	using is_always_equal = true_type;

	constexpr CoTaskMem_allocator() noexcept {}
	constexpr CoTaskMem_allocator(const CoTaskMem_allocator&) noexcept = default;
	template <class _Other>
	constexpr CoTaskMem_allocator(const CoTaskMem_allocator<_Other>&) noexcept {}

	[[nodiscard]]
	constexpr __declspec(allocator) Ty* allocate(const size_t count) const noexcept
	{
		return reinterpret_cast<Ty*>(CoTaskMemAlloc(sizeof(Ty) * count));
	}

	constexpr void deallocate(Ty* const ptr, const size_t count) const noexcept
	{
		CoTaskMemFree(ptr);
	}
};

using vector_LPITEMIDLIST = vector<LPITEMIDLIST, CoTaskMem_allocator<LPITEMIDLIST>>;

vector_LPITEMIDLIST StlGetEnumIDListItems(IEnumIDList* p)
{
	if (p == nullptr) {
		return {};
	}
	LPITEMIDLIST pidl;
	ULONG fetched;
	vector_LPITEMIDLIST pidls;
	while (p->Next(1, &pidl, &fetched) == S_OK) {
		pidls.emplace_back(pidl);
	}
	return pidls;
}

int main()
{
	unique_com_ptr<IShellFolder> desktop;
	if (auto hr = SHGetDesktopFolder(com_ptr_address(desktop)); FAILED(hr)) {
		return hr;
	}

	// デスクトップの表示名を取得
	wstring desktopDisplayName;
	if (auto hr = desktop->GetDisplayNameOf(nullptr, SHGDN_NORMAL, STRRETToWStringWrapper(desktopDisplayName));
		FAILED(hr)) {
		return hr;
	}

	// デスクトップの項目を取得
	unique_com_ptr<IEnumIDList> enumIdList;
	if (auto hr = desktop->EnumObjects(nullptr,
		SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN,
		com_ptr_address(enumIdList));
		FAILED(hr)) {
		return hr;
	}
	auto pidls = StlGetEnumIDListItems(enumIdList.get());

	// デスクトップの項目の名前を取得
	vector<wstring> displayNames;
	transform(pidls.cbegin(), pidls.cend(), back_insert_iterator(displayNames),
		[&desktop](const auto& pidl) {
			wstring displayName;
			desktop->GetDisplayNameOf(pidl, SHGDN_NORMAL, STRRETToWStringWrapper(displayName));
			return displayName;
		});
	for (const auto& displayName : displayNames) {
		wcout << displayName << endl;
	}

	return 0;
}
