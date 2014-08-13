/*
** File Name: processor_query.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z107EBA55_0D42_4C6A_9B3E_247F273597DE
#define Z107EBA55_0D42_4C6A_9B3E_247F273597DE

#include <array>
#include <cstring>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <ccbase/error.hpp>
#include <ctop/cpuid.hpp>
#include <ctop/cpuid_error.hpp>
#include <ctop/system.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	// For `sysconf`.
	#include <unistd.h>
	// For `CPU_SET`.
	#include <sched.h>
#else
	#error "Unsupported kernel."
#endif

namespace ctop {

cc::expected<void>
get_basic_info(processor_package& pkg)
{
	auto buf = std::array<char, 13>{};
	auto regs = (uint32_t*)buf.data();
	auto& eax = regs[0];
	auto& ebx = regs[1];
	auto& ecx = regs[2];
	auto& edx = regs[3];

	/*
	** Retrieve the vendor information.
	*/
	auto vendor_str = boost::string_ref{buf.data() + 4, 12};
	std::tie(eax, ebx, edx, ecx) = cpuid(cpuid_leaf::basic_info);

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
	std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::version_info);

	auto stepping   = eax & 0xF;
	auto model      = (eax >> 4) & 0xF;
	auto family     = (eax >> 8) & 0xF;
	auto type       = (eax >> 12) & 0x3;
	auto ext_model  = (eax >> 16) & 0xF;
	auto ext_family = (eax >> 20) & 0xFF;

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
	** This method for determining whether the processor supports SMT is
	** unreliable, because it does not report whether hyperthreading was
	** disabled in the BIOS. Some propcessors also output false information.
	**
	** Retrieve the information about physical and logical processor count.
	**
	** static constexpr auto htt_bit = uint32_t{1 << 28};
	** pkg.uses_smt(edx & htt_bit);
	*/

	/*
	** This method is unreliable, because different caches could potentially
	** have different line sizes.
	**
	** pkg.cache_line_size(8 * ((ebx >> 8) & 0xFF));
	*/

	/*
	** This method reports the number of slots reserved for APIC IDs, which
	** does not necessarily correspond to the maximum number of logical
	** processors. For example, it can return 16 when the actual number of
	** logical processors is 8.
	**
	** if (htt_supported) {
	** 	auto max_ids = (ebx >> 16) & 0xFF;
	** 	pkg.max_threads(max_ids);
	** 	pkg.max_cores(max_ids / 2);
	** }
	** else {
	** 	pkg.max_threads(1);
	** 	pkg.max_cores(1);
	** }
	*/

	/*
	** Retrieve the brand information and the base frequency.
	*/
	std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::max_extended_leaf);
	if (eax < 0x80000004u) {
		return cpuid_error{cpuid_leaf::brand_string_part_1, "unsupported"};
	}

	auto brand_buf = pkg.version().brand();
	auto p = (uint32_t*)brand_buf.begin();
	std::tie(p[0], p[1], p[2], p[3])   = cpuid(cpuid_leaf::brand_string_part_1);
	std::tie(p[4], p[5], p[6], p[7])   = cpuid(cpuid_leaf::brand_string_part_2);
	std::tie(p[8], p[9], p[10], p[11]) = cpuid(cpuid_leaf::brand_string_part_3);

	/*
	** Get rid of the leading spaces in the brand string.
	*/
	auto beg = brand_buf.find_first_not_of(' ');
	if (beg == boost::string_ref::npos) {
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string is blank"};
	}
	if (brand_buf.back() != '\0') {
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string is not null-terminated"};
	}

	pkg.version().brand_offset(beg);
	auto brand_str = pkg.version().brand();

	/*
	** Extract the base frequency.
	*/
	if (brand_str.length() < 5) {
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string is too small"};
	}

	auto units = brand_str.length() - 4;
	auto first_digit = brand_str.rfind(' ');

	if (first_digit == boost::string_ref::npos) {
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string has no spaces"};
	}
	++first_digit;

	if (first_digit >= units) {
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string has unexpected space"};
	}

	auto freq = brand_str.substr(first_digit, units - first_digit);
	auto sig = double{};
	try {
		sig = boost::lexical_cast<double>(freq);
	}
	catch (const boost::bad_lexical_cast&) {
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"failed to parse base frequency"};
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
		return cpuid_error{cpuid_leaf::brand_string_part_1,
			"failed to parse base frequency"};
	}
	return true;
}

cc::expected<void>
get_cache_info(processor_package& pkg)
{
	auto level = 1;
	uint32_t eax, ebx, ecx, edx;
	std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::enumerable_cache_info, 0);

	for (;;) {
		auto type = eax & 0x1F;
		auto c = cache{};

		switch (type) {
		case 0: return true;
		case 1: c.type(cache_type::data);        break;
		case 2: c.type(cache_type::instruction); break;
		case 3: c.type(cache_type::unified);     break;
		default: return cpuid_error{cpuid_leaf::enumerable_cache_info,
			 "encountered unknown cache type"};
		}

		c.level((eax >> 5) & 0x7);
		c.is_self_initializing((eax >> 8) & 0x1);
		c.is_fully_associative((eax >> 9) & 0x1);
		c.sharing_threads((eax >> 14) & 0xFFF);

		c.line_size((ebx & 0xFFF) + 1);
		c.line_partitions(((ebx >> 12) & 0x3FF) + 1);
		c.associativity(((ebx >> 22) & 0x3FF) + 1);
		c.sets(ecx + 1);
		c.size(c.line_size() * c.line_partitions() * c.associativity() * c.sets());

		c.has_invalidate_propagation(edx & 0x1);
		c.is_inclusive((edx >> 1) & 0x1);
		c.is_direct_mapped((edx >> 2) & 0x1);

		pkg.add_cache(c);
		std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::enumerable_cache_info, level);
		++level;
	}
	return true;
}

cc::expected<void>
get_count_info(processor_package& pkg)
{
	uint32_t eax, ebx, ecx, edx;
	std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::enumerable_topology_info, 0);
	auto count = ebx & 0xFFFF;
	auto type = (ecx >> 8) & 0xFF;

	if (pkg.uses_smt()) {
		if (type != 1 || count != 2) {
			return cpuid_error{cpuid_leaf::enumerable_topology_info,
				"expected level zero to have type \"SMT\" and a "
				"logical processor count of two"};
		}
	}
	else {
		if (type != 2) {
			return cpuid_error{cpuid_leaf::enumerable_topology_info,
				"expected level zero to have type \"core\""};
		}

		pkg.max_cores(count);
		pkg.max_threads(count);
		return true;
	}

	std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::enumerable_topology_info, 1);
	count = ebx & 0xFFFF;
	type = (ecx >> 8) & 0xFF;

	if (type != 2) {
		return cpuid_error{cpuid_leaf::enumerable_topology_info,
			"expected level one to have type \"core\""};
	}
	if (count % 2 != 0) {
		return cpuid_error{cpuid_leaf::enumerable_topology_info,
			"expected logical processor count to be even"};
	}

	pkg.max_threads(count);
	pkg.max_cores(count / 2);
	return true;
}

#if PLATFORM_COMPILER == PLATFORM_COMPILER_GCC || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_CLANG || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_ICC

CC_CONST CC_ALWAYS_INLINE uint32_t 
next_power_of_two(uint32_t x)
{
	return 1 << (32 - __builtin_clz(x - 1));
}

#else
	#error "Unsupported compiler."
#endif

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX

cc::expected<void>
get_topology_info(processor_package& pkg)
{
	// TODO
	return true;
}

#else
	#error "Unsupported kernel."
#endif

cc::expected<processor_package>
system_query()
{
	auto m = max_cpuid_leaf();
	if (!m) {
		return cpuid_unsupported_error{};
	}
	if (m < 11u) {
		return cpuid_error{cpuid_leaf::enumerable_topology_info,
			"unsupported"};
	}

	auto pkg = processor_package{};
	{
		auto r = get_basic_info(pkg);
		if (!r) return r.exception();
	}
	{
		auto r = get_cache_info(pkg);
		if (!r) return r.exception();
	}
	{
		auto r = get_count_info(pkg);
		if (!r) return r.exception();
	}
	{
		auto r = get_topology_info(pkg);
		if (!r) return r.exception();
		return r.get();
	}
}

}

#endif
