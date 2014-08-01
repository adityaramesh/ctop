/*
** File Name: processor_query.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z107EBA55_0D42_4C6A_9B3E_247F273597DE
#define Z107EBA55_0D42_4C6A_9B3E_247F273597DE

#include <algorithm>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <ccbase/error.hpp>
#include <ctop/cpuid.hpp>
#include <ctop/system.hpp>

namespace ctop {

cc::expected<processor_package>
system_query()
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
	auto buf = std::array<char, 13>{};
	auto r = (uint32_t*)buf.data();
	auto vendor_str = boost::string_ref{buf.data() + 4, 12};
	std::tie(r[0], r[1], r[3], r[2]) = cpuid(0);

	if (vendor_str == "GenuineIntel") {
		pkg.vendor(vendor::intel);
	}
	else if (vendor_str == "AuthenticAMD") {
		pkg.vendor(vendor::amd);
	}
	else {
		pkg.vendor(vendor::unknown);
	}

	/*
	** Retrieve the version information.
	*/
	std::tie(r[0], r[1], r[2], r[3]) = cpuid(1);

	auto stepping   = r[0] & 0xF;
	auto model      = (r[0] >> 4) & 0xF;
	auto family     = (r[0] >> 8) & 0xF;
	auto type       = (r[0] >> 12) & 0x3;
	auto ext_model  = (r[0] >> 16) & 0xF;
	auto ext_family = (r[0] >> 20) & 0xFF;

	pkg.version().type(static_cast<processor_type>(type));
	pkg.version().stepping(stepping);

	/*
	** See the instruction reference manual for why we do thipkg.
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
	** Retrieve the information about physical and logical processor countpkg.
	*/
	static constexpr auto htt_bit = uint32_t{1 << 27};
	auto max_ids = (r[1] >> 16) & 0xFF;
	auto htt_supported = r[3] & htt_bit;

	// Different caches could potentially have different line sizes.
	// pkg.cache_line_size(8 * ((r[1] >> 8) & 0xFF));

	if (htt_supported) {
		pkg.max_threads(max_ids);
		pkg.max_cores(max_ids / 2);
	}
	else {
		pkg.max_threads(1);
		pkg.max_cores(1);
	}

	/*
	** Retrieve the brand information and the base frequency.
	*/
	std::tie(r[0], r[1], r[2], r[3]) = cpuid(0x80000000);
	if (r[0] < 0x80000004u) {
		return std::runtime_error{"Brand string not supported."};
	}

	auto brand_buf = pkg.version().brand();
	auto p = (uint32_t*)brand_buf.begin();
	std::tie(p[0], p[1], p[2], p[3])   = cpuid(0x80000002);
	std::tie(p[4], p[5], p[6], p[7])   = cpuid(0x80000003);
	std::tie(p[8], p[9], p[10], p[11]) = cpuid(0x80000004);

	/*
	** Get rid of the leading spaces in the brand string.
	*/
	auto beg = brand_buf.find_first_not_of(' ');
	if (beg == boost::string_ref::npos) {
		return std::runtime_error{"Brand string is blank."};
	}
	if (brand_buf.back() != '\0') {
		return std::runtime_error{"Brand string is not null-terminated."};
	}

	pkg.version().brand_offset(beg);
	auto brand_str = pkg.version().brand();

	/*
	** Extract the base frequency.
	*/
	if (brand_str.length() < 5) {
		return std::runtime_error{"Brand string too small."};
	}

	auto units = brand_str.length() - 4;
	auto first_digit = brand_str.rfind(' ');

	if (first_digit == boost::string_ref::npos) {
		return std::runtime_error{"Brand string has no spaces."};
	}
	++first_digit;

	if (first_digit >= units) {
		return std::runtime_error{"Brand string has unexpected space."};
	}

	auto freq = brand_str.substr(first_digit, units - first_digit);
	auto sig = double{};
	try {
		sig = boost::lexical_cast<double>(freq);
	}
	catch (const boost::bad_lexical_cast&) {
		return std::runtime_error{"Failed to parse base frequency."};
	}

	auto units_str = brand_str.substr(units, 3);
	if (units_str == "MHz") {
		pkg.version().base_frequency(sig);
	}
	else if (units_str == "GHz") {
		pkg.version().base_frequency(1000 * sig);
	}
	else if (units_str == "THz") {
		pkg.version().base_frequency(1000000 * sig);
	}
	else {
		return std::runtime_error{"Could not determine base frequency."};
	}
	return pkg;
}

}

#endif
