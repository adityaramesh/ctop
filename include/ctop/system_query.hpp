/*
** File Name: system_query.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z107EBA55_0D42_4C6A_9B3E_247F273597DE
#define Z107EBA55_0D42_4C6A_9B3E_247F273597DE

#include <array>
#include <algorithm>
#include <stdexcept>
#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include <ccbase/error.hpp>

#include <ctop/cpuid.hpp>
#include <ctop/cpuid_error.hpp>
#include <ctop/numa_error.hpp>
#include <ctop/system.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	#include <unistd.h>
	// For `CPU_SET`.
	#include <sched.h>
#else
	#error "Unsupported kernel."
#endif

namespace ctop {

#if PLATFORM_COMPILER == PLATFORM_COMPILER_GCC   || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_CLANG || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_ICC

CC_CONST CC_ALWAYS_INLINE uint32_t 
roundup_to_pot(uint32_t x)
{
	return 1 << (32 - __builtin_clz(x - 1));
}

#else
	#error "Unsupported compiler."
#endif

void get_basic_cpu_info(global_cpu_info& info)
{
	static const auto _ = std::ignore;

	std::array<char, 13> buf;
	buf[12] = '\0';

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
		info.version().vendor(cpu_vendor::intel);
	}
	else if (vendor_str == "AuthenticAMD") {
		info.version().vendor(cpu_vendor::amd);
	}
	else {
		info.version().vendor(cpu_vendor::unknown);
	}

	/*
	** Retrieve the version information.
	*/
	std::tie(eax, ebx, _, edx) = cpuid(cpuid_leaf::version_info);

	auto stepping   = eax & 0xF;
	auto model      = (eax >> 4) & 0xF;
	auto family     = (eax >> 8) & 0xF;
	auto type       = (eax >> 12) & 0x3;
	auto ext_model  = (eax >> 16) & 0xF;
	auto ext_family = (eax >> 20) & 0xFF;

	info.version().type(static_cast<cpu_type>(type));
	info.version().stepping(stepping);

	/*
	** See the instruction reference manual for why we do do not directly
	** store the values reported by CPUID.
	*/
	if (family != 0xF) {
		info.version().family(family);
	}
	else {
		info.version().family(ext_family + family);
	}

	if (family == 0x6 || family == 0xF) {
		info.version().model((ext_model << 4) + model);
	}
	else {
		info.version().model(model);
	}

	auto htt_bit = edx & (1 << 28);
	if (htt_bit) {
		info.thread_ids_per_package(roundup_to_pot((ebx >> 16) & 0xFF));
	}
	else {
		info.thread_ids_per_package(1);
	}

	/*
	** Retrieve the brand information and the base frequency.
	*/
	std::tie(eax, _, _, _) = cpuid(cpuid_leaf::max_extended_leaf);
	if (eax < 0x80000004u) {
		throw cpuid_error{cpuid_leaf::brand_string_part_1, "unsupported"};
	}

	auto brand_buf = info.version().brand();
	auto p = (uint32_t*)brand_buf.begin();
	std::tie(p[0], p[1], p[2], p[3])   = cpuid(cpuid_leaf::brand_string_part_1);
	std::tie(p[4], p[5], p[6], p[7])   = cpuid(cpuid_leaf::brand_string_part_2);
	std::tie(p[8], p[9], p[10], p[11]) = cpuid(cpuid_leaf::brand_string_part_3);

	/*
	** Get rid of the leading spaces in the brand string.
	*/
	auto beg = brand_buf.find_first_not_of(' ');
	if (beg == boost::string_ref::npos) {
		throw cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string is blank"};
	}
	if (brand_buf.back() != '\0') {
		throw cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string is not null-terminated"};
	}

	info.version().brand_offset(beg);
	auto brand_str = info.version().brand();

	/*
	** Extract the base frequency.
	*/
	if (brand_str.length() < 5) {
		throw cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string is too small"};
	}

	auto units = brand_str.length() - 4;
	auto first_digit = brand_str.rfind(' ');

	if (first_digit == boost::string_ref::npos) {
		throw cpuid_error{cpuid_leaf::brand_string_part_1,
			"brand string has no spaces"};
	}
	++first_digit;

	if (first_digit >= units) {
		throw cpuid_error{cpuid_leaf::brand_string_part_2,
			"brand string has unexpected space"};
	}

	auto freq = brand_str.substr(first_digit, units - first_digit);
	auto sig = double{};
	try {
		sig = boost::lexical_cast<double>(freq);
	}
	catch (const boost::bad_lexical_cast&) {
		throw cpuid_error{cpuid_leaf::brand_string_part_1,
			"failed to parse base frequency"};
	}

	auto units_str = brand_str.substr(units, 3);
	if (units_str == "MHz") {
		info.version().base_frequency(sig);
	}
	else if (units_str == "GHz") {
		info.version().base_frequency(1000 * sig);
	}
	else if (units_str == "THz") {
		info.version().base_frequency(1000000 * sig);
	}
	else {
		throw cpuid_error{cpuid_leaf::brand_string_part_1,
			"failed to parse base frequency"};
	}
	return cc::no_error;
}

void get_cpu_layout_info(global_cpu_info& info)
{
	static constexpr auto leaf = cpuid_leaf::enumerable_topology_info;
	static const auto _ = std::ignore;
	uint32_t eax, ebx, ecx;
	uint32_t smt_count;
	auto level = 0;
	auto checked = 0;

	for (;;) {
		std::tie(eax, ebx, ecx, _) = cpuid(leaf, level);
		auto shift = eax & 0x1F;
		auto count = ebx & 0xFFFF;
		auto type  = (ecx >> 8) & 0xFF;

		if (type == 0) {
			break;
		}
		else if (count == 0) {
			throw cpuid_error{leaf, "obtained logical processor "
				"count of zero"};
		}

		if (type == 1) {
			info.smt_id_bits(shift);
			smt_count = count;
			if (smt_count > 2) {
				throw cpuid_error{leaf, "obtained count of "
					"more than two SMTs per core"};
			}
			++checked;
		}
		else if (type == 2) {
			info.core_id_bits(shift);
			info.total_threads(count);
			++checked;
		}
		else {
			throw cpuid_error{leaf, "unknown level type"};
		}
		++level;
	}

	if (checked != 2) {
		throw cpuid_error{leaf, "did not encounter both levels one "
			"and two"};
	}
	if (info.smt_id_bits() + info.core_id_bits() > 32) {
		throw cpuid_error{leaf, "sub-ID shift widths sum to number "
			"greater than 32"};
	}
	if (smt_count > info.total_threads()) {
		throw cpuid_error{leaf, "SMT count greater than thread count"};
	}
	if (info.total_threads() % smt_count != 0) {
		throw cpuid_error{leaf, "total thread count not divisible by "
			"number of threads per core"};
	}
	info.package_id_bits(32 - info.smt_id_bits() - info.core_id_bits());
	info.total_cores(info.total_threads() / smt_count);
}

void get_cpu_cache_info(global_cpu_info& info)
{
	static constexpr auto leaf = cpuid_leaf::enumerable_cache_info;
	auto level = 1;
	uint32_t eax, ebx, ecx, edx;
	std::tie(eax, ebx, ecx, edx) = cpuid(leaf, 0);

	for (;;) {
		auto type = eax & 0x1F;
		auto c = cpu_cache{};

		switch (type) {
		case 0: return;
		case 1: c.type(cache_type::data);        break;
		case 2: c.type(cache_type::instruction); break;
		case 3: c.type(cache_type::unified);     break;
		default: throw cpuid_error{leaf, "encountered unknown cache type"};
		}

		c.level((eax >> 5) & 0x7);
		c.is_self_initializing((eax >> 8) & 0x1);
		c.is_fully_associative((eax >> 9) & 0x1);

		info.core_ids_per_package(roundup_to_pot((eax >> 26) + 1));
		if (info.thread_ids_per_package() < info.core_ids_per_package()) {
			throw cpuid_error{leaf, "fewer thread IDs per package "
				"than core IDs per package"};
		}
		else if (info.thread_ids_per_package() % info.core_ids_per_package() != 0) {
			throw cpuid_error{leaf, "number of thread IDs per "
				"package not a multiple of number of core "
				"IDs per package"};
		}

		auto sharing_ids = roundup_to_pot(((eax >> 14) & 0xFFF) + 1);
		if (sharing_ids == info.thread_ids_per_core()) {
			c.scope(cpu_topology_level::core);
		}
		else if (sharing_ids == info.thread_ids_per_package()) {
			c.scope(cpu_topology_level::processor);
		}
		else {
			throw cpuid_error{leaf, "failed to determine scope of cache"};
		}

		c.line_size((ebx & 0xFFF) + 1);
		c.line_partitions(((ebx >> 12) & 0x3FF) + 1);
		c.associativity(((ebx >> 22) & 0x3FF) + 1);
		c.sets(ecx + 1);
		c.size(c.line_size() * c.line_partitions() * c.associativity() * c.sets());

		c.has_invalidate_propagation(edx & 0x1);
		c.is_inclusive((edx >> 1) & 0x1);
		c.is_direct_mapped((edx >> 2) & 0x1);

		info.add(c);
		std::tie(eax, ebx, ecx, edx) = cpuid(cpuid_leaf::enumerable_cache_info, level);
		++level;
	}
}

void get_global_info(system_info& info)
{
	auto& cpu = info.cpu_info();
	get_basic_cpu_info(cpu);
	get_cpu_layout_info(cpu);
	get_cpu_cache_info(cpu);
}

void get_numa_inventory(system_info& info)
{
	if (::numa_available() == -1) {
		throw numa_error{"libnuma unavailable"};
	}

	auto count = 0;

	count = ::numa_num_configured_nodes();
	if (count <= 0) {
		throw numa_error{"failed to get total NUMA node count"};
	}
	info.total_numa_nodes(count);

	count = ::numa_num_task_nodes();
	if (count <= 0) {
		throw numa_error{"failed to get available NUMA node count"};
	}
	if ((uint32_t)count > info.total_numa_nodes()) {
		throw numa_error{"libnuma reports more available NUMA nodes "
			"than configured NUMA nodes"};
	}
	info.available_numa_nodes(count);

	if ((uint32_t)::numa_num_configured_cpus() != info.total_cpu_threads()) {
		throw numa_error{"total CPU count disagrees with information "
			"reported by libnuma"};
	}

	count = ::numa_num_task_cpus();
	if (count <= 0) {
		throw numa_error{"failed to get available CPU thread count"};
	}
	if ((uint32_t)count > info.total_cpu_threads()) {
		throw numa_error{"libnuma reports more available CPU threads "
			"than total CPU threads"};
	}
	info.available_cpu_threads(count);
}

void get_cpu_topology_info(
	uint32_t&       cur_thread_count,
	struct bitmask* cpus,
	struct bitmask* cur_cpu,
	numa_node_info& node,
	system_info&    info
)
{
	static constexpr auto leaf = cpuid_leaf::enumerable_topology_info;
	static const auto _ = std::ignore;

	if (::numa_node_to_cpus(node.id(), cpus) == -1) {
		if (errno == ERANGE) {
			throw numa_error{node.id(), "node contains more CPU "
				"threads than expected"};
		}
		else {
			auto msg = cc::format("failed to get CPU thread IDs of "
				"node: $", std::strerror(errno));
			throw numa_error{node.id(), std::move(msg)};
		}
	}

	auto avail_threads = ::numa_bitmask_weight(cpus);
	if (avail_threads == 0) {
		throw numa_error{node.id(), "node reported accessible but "
			"contains no usable CPU threads"};
	}
	if (cur_thread_count + avail_threads > info.available_cpu_threads().size()) {
		throw numa_error{"more CPU threads available than reported"};
	}

	auto& cpu = node.cpu_info();
	cpu.available_threads(avail_threads);
	cpu.thread_data(&info.available_cpu_threads()[cur_thread_count]);
	cur_thread_count += avail_threads;

	for (auto i = 0u, cur_thread = 0u; i != info.total_cpu_threads(); ++i) {
		if (::numa_bitmask_isbitset(cpus, i)) {
			::numa_bitmask_setbit(cur_cpu, i);
			if (::numa_sched_setaffinity(0, cur_cpu) == -1) {
				auto msg = cc::format("failed to schedule "
					"thread on CPU $: $", i,
					std::strerror(errno));
				throw numa_error{node.id(), msg};
			}
			::numa_bitmask_clearbit(cur_cpu, i);

			auto& thread = cpu.available_threads()[cur_thread++];
			thread.os_id(i);
			std::tie(_, _, _, thread.x2apic_id()) = cpuid(leaf, 0);
		}
	}

	boost::sort(cpu.available_threads(),
		[](const cpu_thread_info& lhs, const cpu_thread_info& rhs) {
			throw lhs.x2apic_id() < rhs.x2apic_id();
		});

	if (cpu.available_threads().size() >= 2) {
		for (auto i = 0u; i != cpu.available_threads().size() - 1; ++i) {
			if (cpu.available_threads()[i].x2apic_id() ==
				cpu.available_threads()[i + 1].x2apic_id())
			{
				throw numa_error{node.id(), "detected "
					"duplicate x2APIC IDs that should "
					"have been obtained from different "
					"hardware threads"};
			}
		}
	}

	auto total_cores = info.cpu_info().total_cores();
	auto unique_cores = count_unique_cores(cpu.available_threads(),
		info.cpu_info());

	cpu.uses_smt(avail_threads > total_cores ||
		unique_cores < avail_threads);
}

void get_numa_topology_info(system_info& info)
{
	auto max_node = ::numa_max_possible_node();
	if (max_node <= 0) {
		throw numa_error{"failed to get maximum NUMA node number"};
	}

	auto nodes = ::numa_all_nodes_ptr;
	auto cpus = ::numa_bitmask_alloc(info.total_cpu_threads());
	auto cur_cpu = ::numa_bitmask_alloc(info.total_cpu_threads());

	BOOST_SCOPE_EXIT_ALL(&) {
		if (cpus != nullptr) {
			::numa_bitmask_free(cpus);
		}
		if (cur_cpu != nullptr) {
			::numa_bitmask_free(cur_cpu);
		}
	};

	if (::numa_bitmask_weight(nodes) != info.available_numa_nodes().size()) {
		throw numa_error{"conflicting available node counts"};
	}
	if (cpus == nullptr || cur_cpu == nullptr) {
		throw numa_error{"failed to allocate bitmask"};
	}

	auto cur_thread_count = uint32_t{};
	for (auto i = 0u, cur_node = 0u; i != (unsigned)max_node; ++i) {
		if (::numa_bitmask_isbitset(nodes, i)) {
			auto& node = info.available_numa_nodes()[cur_node++].id(i);
			get_cpu_topology_info(cur_thread_count, cpus, cur_cpu,
				node, info);
		}
	}

	if (cur_thread_count != info.available_cpu_threads().size()) {
		throw numa_error{"number of CPU threads actually accessible "
			"does not match reported count"};
	}
}

void get_numa_info(system_info& info)
{
	get_numa_inventory(info);
	get_numa_topology_info(info);
}

cc::expected<system_info>
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

	return cc::attempt([]() {
		auto info = system_info{};
		get_global_info(info);
		get_numa_info(info);
		return info;
	});
}

}

#endif
