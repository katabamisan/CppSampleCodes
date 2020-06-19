// SID <-> STL String

#include <string>
#include <string_view>
#include <vector>

#define STRICT
#define NOMINMAX
#include <Windows.h>
#include <sddl.h>

using namespace std;

tuple<SID, wstring, SID_NAME_USE> StlLookupAccountName(wstring_view systemName, wstring_view accountName)
{
	DWORD refDomainNameSize = 0;
	DWORD sidSize = 0;
	SID_NAME_USE nameUse;
	LookupAccountNameW(systemName.data(), accountName.data(), nullptr,
		&sidSize, nullptr, &refDomainNameSize, &nameUse);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		return {};
	}
	wstring refDomainName(refDomainNameSize, L'\0');
	SID sid;
	SID_NAME_USE nameUse;
	if (!LookupAccountNameW(systemName.data(), accountName.data(), &sid,
		&sidSize, refDomainName.data(), &refDomainNameSize, &nameUse)) {
		return {};
	}
	return {sid, move(refDomainName), nameUse};
}

wstring StlConvertSIDToStringW(PSID psid)
{
	LPWSTR psz;
	wstring ret;
	if (ConvertSidToStringSidW(psid, &psz))
	{
		ret = psz;
		LocalFree(psz);
	}
	return ret;
}

tuple<wstring, wstring, SID_NAME_USE> StlLookupAccountSid(wstring_view systemName, PSID psid)
{
	DWORD nameSize = 0;
	DWORD refDomainNameSize = 0;
	SID_NAME_USE sidNameUse;
	LookupAccountSidW(systemName.data(), psid, nullptr, &nameSize, nullptr, &refDomainNameSize, &sidNameUse);
	if (auto lr = GetLastError(); lr != ERROR_INSUFFICIENT_BUFFER) {
		return {};
	}
	wstring name(nameSize, L'\0');
	wstring refDomainName(refDomainNameSize, L'\0');
	if (!LookupAccountSidW(systemName.data(), psid, name.data(), &nameSize, refDomainName.data(), &refDomainNameSize, &sidNameUse))
	{
		return {};
	}
	return {move(name), move(refDomainName), sidNameUse};
}

int main()
{
	auto [sid, referencedDomainName, sidNameUse] = StlLookupAccountName({}, {});
	auto sidStr = StlConvertSIDToStringW(&sid);
	auto [name, referencedDomainName2, sidNameUse2] = StlLookupAccountSid({}, &sid);

	return 0;
}
