/*
** File Name: basic_info.cpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#include <array>
#include <cstdint>
#include <tuple>
#include <cpuid.h>
#include <ccbase/format.hpp>

auto get_cpuid(uint32_t level)
{
	uint32_t r1, r2, r3, r4;
	__cpuid(level, r1, r2, r3, r4);
	return std::make_tuple(r1, r2, r3, r4);
}

int main()
{
	auto m = __get_cpuid_max(0, nullptr);
	if (m == 0) {
		cc::println("CPUID unsupported.");
		return EXIT_FAILURE;
	}
	cc::println("Maximum supported CPUID level: $.", m);

	auto buf = std::array<uint8_t, 33>{};
	auto r = (uint32_t*)buf.data();

	std::tie(r[0], r[1], r[3], r[2]) = get_cpuid(0);
	cc::println("Vendor ID: \"$\".", (char*)&r[1]);

	std::tie(r[0], r[1], r[2], r[3]) = get_cpuid(1);
	auto brand_index     = r[1] & 0xFF;
	auto cache_line_size = 8 * ((r[1] >> 8) & 0xFF);
	auto max_ids         = (r[1] >> 16) & 0xFF;
	auto apic_id         = r[1] >> 24;

	static constexpr auto htt_bit = uint32_t{1 << 27};
	auto htt_supported = r[3] & htt_bit;

	cc::println("Brand index: $.", brand_index);
	cc::println("Cache line size: $ bytes.", cache_line_size);
	if (htt_supported) {
		cc::println("Number of logical processors: $.", max_ids);
	}
	else {
		cc::println("Number of logical processors: 1.");
	}

	// TODO: determine the APIC ID of each logical processor. How about NUMA
	// affinity?
	cc::println("APIC ID: $.", apic_id);
}
