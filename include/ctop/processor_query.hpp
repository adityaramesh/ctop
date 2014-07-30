/*
** File Name: processor_query.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z107EBA55_0D42_4C6A_9B3E_247F273597DE
#define Z107EBA55_0D42_4C6A_9B3E_247F273597DE

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ccbase/error.hpp>
#include <ctop/processor.hpp>
#include <ctop/system.hpp>

namespace ctop {

cc::expected<processor_package>
processor_description()
{
	auto m = max_cpuid_leaf();
	if (!m) {
		return std::runtime_error{"CPUID unsupported."};
	}
	if (m < 11u) {
		return std::runtime_error{"CPUID leaf 11 unsupported."};
	}

	auto pkg = processor_package{};

	/*
	** Retrieve the vendor information.
	*/
	auto buf = std::array<char, 33>{};
	auto r = (uint32_t*)buf.data();
	std::tie(r[0], r[1], r[3], r[2]) = get_cpuid(0);

	if (std::strcmp((char*)&r[1], "GenuineIntel") == 0) {
		pkg.vendor(vendor::intel);
	}
	else if (std::strcmp((char*)&r[1], "AuthenticAMD") == 0) {
		pkg.vendor(vendor::amd);
	}
	else {
		pkg.vendor(vendor::unknown);
	}

	/*
	** Retrieve the version information.
	*/
	std::tie(r[0], r[1], r[2], r[3]) = get_cpuid(1);

	auto stepping   = r[0] & 0xF;
	auto model      = (r[0] >> 4) & 0xF;
	auto family     = (r[0] >> 8) & 0xF;
	auto type       = (r[0] >> 12) & 0x3;
	auto ext_model  = (r[0] >> 16) & 0xF;
	auto ext_family = (r[0] >> 20) & 0xFF;

	pkg.version().type(static_cast<processor_type>(type));
	pkg.version().stepping(stepping);

	/*
	** See the instruction reference manual for why we do this.
	*/
	if (family != 0xF) {
		pkg.version().family(family);
	}
	else {
		pkg.version().family(ext_family + family);
	}

	if (family == 0x6 || family == 0xF) {
		pkg.version().model((ext_model << 4) + model);
	}
	else {
		pkg.version().model(model);
	}

	/*
	** Retrieve the information about physical and logical processor counts.
	*/
	static constexpr auto htt_bit = uint32_t{1 << 27};
	auto max_ids = (r[1] >> 16) & 0xFF;
	auto htt_supported = r[3] & htt_bit;
	pkg.cache_line_size(8 * ((r[1] >> 8) & 0xFF));

	if (htt_supported) {
		pkg.logical_processors(max_ids);
		pkg.physical_processors(max_ids / 2);
	}
	else {
		pkg.logical_processors(1);
		pkg.physical_processors(1);
	}

	/*
	** Retrieve the brand information and the base frequency.
	*/
	std::tie(r[0], r[1], r[2], r[3]) = get_cpuid(0x80000000);
	if (r[0] < 0x80000004u) {
		return std::runtime_error{"Brand string not supported."};
	}

	auto p = (uint32_t*)pkg.version().brand();
	std::tie(p[0], p[1], p[2], p[3]) = get_cpuid(0x80000002);
	std::tie(p[4], p[5], p[6], p[7]) = get_cpuid(0x80000003);
	std::tie(p[8], p[9], p[10], p[11]) = get_cpuid(0x80000004);

	/*
	** Get rid of the leading spaces in the brand string.
	*/
	auto f = (char*)p;
	auto l = (char*)p + 48;
	auto it = std::find_if_not(f, l, [](auto x) { return x == ' '; });
	if (it == l) {
		return std::runtime_error{"Brand string is blank."};
	}
	std::copy(it, l, f);

	auto units = l - 4;
	auto last_space = units - 1;
	for (auto i = 0; i != 44; ++i) {
		if (*last_space == ' ') {
			break;
		}
		--last_space;
	}

	auto sig = std::strtod(last_space + 1, nullptr);
	if (sig == 0.0) {
		return std::runtime_error{"Failed to parse base frequency."};
	}

	if (std::strcmp(units, "MHz") == 0) {
		pkg.version().base_frequency(sig);
	}
	else if (std::strcmp(units, "GHz") == 0) {
		pkg.version().base_frequency(1000 * sig);
	}
	else if (std::strcmp(units, "THz") == 0) {
		pkg.version().base_frequency(1000000 * sig);
	}
	else {
		return std::runtime_error{"Could not determine base frequency."};
	}
	return pkg;
}

}

#endif
