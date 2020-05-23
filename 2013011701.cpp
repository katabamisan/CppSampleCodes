//
// インラインアセンブリでCPUID情報を取得する
//
// このコードはx86コードでしか動作しません。
x64ではintrin.hの__cpuid、__cpuidexを使用してください。
//

#include <iostream>
#include <iomanip>
#include <string>

namespace cpuid {
	void cpuid(int in_eax, int& ret_eax, int& ret_ebx, int& ret_ecx, int& ret_edx);
	int get_largest_standard_function_number_supported();
	int get_largest_extended_function_number_supported();
	std::string get_vender_id();
	std::string get_processor_brand_string();
}

int main()
{
	std::cout << "標準関数の最大番号：0x" << std::hex << std::setw(8) << std::setfill('0') << cpuid::get_largest_standard_function_number_supported() << std::endl;
	std::cout << "拡張関数の最大番号：0x" << std::hex << std::setw(8) << std::setfill('0') << cpuid::get_largest_extended_function_number_supported() << std::endl;
	std::cout << "ベンダーID：" << cpuid::get_vender_id() << std::endl;
	std::cout << "プロセッサーのブランド文字列：" << cpuid::get_processor_brand_string() << std::endl;

	return 0;
}

void cpuid::cpuid(int in_eax, int& out_eax, int& out_ebx, int& out_ecx, int& out_edx)
{
	int a, b, c, d;
	__asm {
		mov eax, in_eax
		cpuid
		mov a, eax
		mov b, ebx
		mov c, ecx
		mov d, edx
	}
	out_eax = a;
	out_ebx = b;
	out_ecx = c;
	out_edx = d;
}

int cpuid::get_largest_standard_function_number_supported()
{
	int eax, ebx, ecx, edx;
	cpuid(0, eax, ebx, ecx, edx);
	return eax;
}

int cpuid::get_largest_extended_function_number_supported()
{
	int eax, ebx, ecx, edx;
	cpuid(0x80000000, eax, ebx, ecx, edx);
	return eax;
}

std::string cpuid::get_vender_id()
{
	int eax, ebx, ecx, edx;
	cpuid(0, eax, ebx, ecx, edx);

	char vender_id[13];
	*reinterpret_cast<int*>(vender_id) = ebx;
	*reinterpret_cast<int*>(vender_id + 4) = edx;
	*reinterpret_cast<int*>(vender_id + 8) = ecx;
	vender_id[12] = 0;

	return vender_id;
}

std::string cpuid::get_processor_brand_string()
{
	char processor_brand_string[4 * 4 * 3 + 1];
	int eax, ebx, ecx, edx;
	cpuid(0x80000002, eax, ebx, ecx, edx);
	*reinterpret_cast<int*>(processor_brand_string) = eax;
	*reinterpret_cast<int*>(processor_brand_string + 4) = ebx;
	*reinterpret_cast<int*>(processor_brand_string + 8) = ecx;
	*reinterpret_cast<int*>(processor_brand_string + 12) = edx;
	cpuid(0x80000003, eax, ebx, ecx, edx);
	*reinterpret_cast<int*>(processor_brand_string + 16) = eax;
	*reinterpret_cast<int*>(processor_brand_string + 20) = ebx;
	*reinterpret_cast<int*>(processor_brand_string + 24) = ecx;
	*reinterpret_cast<int*>(processor_brand_string + 28) = edx;
	cpuid(0x80000004, eax, ebx, ecx, edx);
	*reinterpret_cast<int*>(processor_brand_string + 32) = eax;
	*reinterpret_cast<int*>(processor_brand_string + 36) = ebx;
	*reinterpret_cast<int*>(processor_brand_string + 40) = ecx;
	*reinterpret_cast<int*>(processor_brand_string + 44) = edx;
	processor_brand_string[48] = 0;
	return processor_brand_string;
}
