/*
** File Name: system.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z81EA1B11_A653_467B_BFC2_F5F532D5D3F8
#define Z81EA1B11_A653_467B_BFC2_F5F532D5D3F8

#include <array>
#include <vector>
#include <ostream>

#include <boost/range/iterator_range.hpp>
#include <boost/utility/string_ref.hpp>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
	#include <numa.h>
#else
	#error "Unsupported platform kernel."
#endif

namespace ctop {

enum class cpu_vendor : uint8_t
{
	intel,
	amd,
	unknown,
};

std::ostream& operator<<(std::ostream& os, const cpu_vendor& v)
{
	switch (v) {
	case cpu_vendor::intel:
		cc::write(os, "Intel");
		return os;
	case cpu_vendor::amd:
		cc::write(os, "AMD");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

enum class cpu_type : uint8_t
{
	original_oem    = 0x0,
	intel_overdrive = 0x1,
	dual_processor  = 0x2,
	reserved        = 0x3,
};

std::ostream& operator<<(std::ostream& os, const cpu_type& t)
{
	switch (t) {
	case cpu_type::original_oem:
		cc::write(os, "Original OEM");
		return os;
	case cpu_type::intel_overdrive:
		cc::write(os, "Intel OverDrive");
		return os;
	case cpu_type::dual_processor:
		cc::write(os, "Dual Processor");
		return os;
	case cpu_type::reserved:
		cc::write(os, "Reserved");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

class cpu_version final
{
	static constexpr auto brand_length = 48;

	double m_base_freq_mhz;
	cpu_vendor m_vendor;
	cpu_type m_type;
	uint8_t m_brand_off{};
	uint8_t m_family;
	uint8_t m_model;
	uint8_t m_stepping;
	std::array<char, brand_length> m_brand;
public:
	explicit cpu_version() noexcept {}

	boost::string_ref brand()
	{
		return {
			m_brand.data() + m_brand_off,
			(size_t)(brand_length - m_brand_off)
		};
	}

	const boost::string_ref brand() const
	{
		return {
			m_brand.data() + m_brand_off,
			(size_t)(brand_length - m_brand_off)
		};
	}

	DEFINE_COPY_GETTER_SETTER(cpu_version, base_frequency, m_base_freq_mhz)
	DEFINE_COPY_GETTER_SETTER(cpu_version, vendor, m_vendor)
	DEFINE_COPY_GETTER_SETTER(cpu_version, type, m_type)
	DEFINE_COPY_GETTER_SETTER(cpu_version, brand_offset, m_brand_off)
	DEFINE_COPY_GETTER_SETTER(cpu_version, family, m_family)
	DEFINE_COPY_GETTER_SETTER(cpu_version, model, m_model)
	DEFINE_COPY_GETTER_SETTER(cpu_version, stepping, m_stepping)
};

std::ostream& operator<<(std::ostream& os, const cpu_version& v)
{
	cc::write(os, v.brand());
	return os;
}

enum class cpu_topology_level : uint8_t
{
	thread,
	core,
	processor,
};

std::ostream& operator<<(std::ostream& os, const cpu_topology_level& t)
{
	switch (t) {
	case cpu_topology_level::thread:
		cc::write(os, "thread");
		return os;
	case cpu_topology_level::core:
		cc::write(os, "core");
		return os;
	case cpu_topology_level::processor:
		cc::write(os, "processor");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

/*
** Note that this enum's name is not prefixed by `cpu_`, because we could
** potentially use this enum to describe the cache types of several different
** kinds of processors.
*/
enum class cache_type : uint8_t
{
	instruction,
	data,
	unified,
};

std::ostream& operator<<(std::ostream& os, const cache_type& t)
{
	switch (t) {
	case cache_type::instruction:
		cc::write(os, "instruction");
		return os;
	case cache_type::data:
		cc::write(os, "data");
		return os;
	case cache_type::unified:
		cc::write(os, "unified");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

class cpu_cache final
{
	cache_type m_type;
	bool m_self_init;
	bool m_fully_assoc;
	bool m_inv_propagation;
	bool m_inclusive;
	bool m_direct;

	cpu_topology_level m_scope;
	uint8_t m_level;
	uint32_t m_size;
	uint32_t m_sets;
	uint32_t m_line_size;
	uint32_t m_line_partitions;
	uint32_t m_assoc;
public:
	explicit cpu_cache() noexcept {}

	DEFINE_COPY_GETTER_SETTER(cpu_cache, type, m_type)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, is_self_initializing, m_self_init)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, is_fully_associative, m_fully_assoc)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, has_invalidate_propagation, m_inv_propagation)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, is_inclusive, m_inv_propagation)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, is_direct_mapped, m_direct)

	DEFINE_COPY_GETTER_SETTER(cpu_cache, scope, m_scope)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, level, m_level)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, size, m_size)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, sets, m_sets)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, line_size, m_line_size)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, line_partitions, m_line_partitions)
	DEFINE_COPY_GETTER_SETTER(cpu_cache, associativity, m_assoc)
};

std::ostream& operator<<(std::ostream& os, const cpu_cache& c)
{
	cc::write(os, "L${num} $ cache (${data}, per $)",
		c.level(), c.type(), c.size(), c.scope());
	return os;
}

class cpu_thread_info final
{
	uint32_t m_os_id;
	uint32_t m_x2apic_id;
public:
	explicit cpu_thread_info() noexcept {}

	DEFINE_REF_GETTER_SETTER(cpu_thread_info, os_id, m_os_id)
	DEFINE_REF_GETTER_SETTER(cpu_thread_info, x2apic_id, m_x2apic_id)
};

std::ostream& operator<<(std::ostream& os, const cpu_thread_info& i)
{
	cc::write(os, "CPU thread: {x2APIC ID: $, OS ID: $}",
		i.x2apic_id(), i.os_id());
	return os;
}

class local_cpu_info final
{
	cpu_thread_info* m_thread_data;
	uint8_t m_avail_threads;
	uint8_t m_uses_smt;

	using thread_range       = boost::iterator_range<cpu_thread_info*>;
	using const_thread_range = boost::iterator_range<const cpu_thread_info*>;
public:
	explicit local_cpu_info() noexcept {}

	thread_range available_threads() noexcept
	{ return {m_thread_data, m_thread_data + m_avail_threads}; }

	const_thread_range available_threads() const noexcept
	{ return {m_thread_data, m_thread_data + m_avail_threads}; }

	DEFINE_SETTER(local_cpu_info, thread_data, m_thread_data)
	DEFINE_SETTER(local_cpu_info, available_threads, m_avail_threads)
	DEFINE_COPY_GETTER_SETTER(local_cpu_info, uses_smt, m_uses_smt)
};

std::ostream& operator<<(std::ostream& os, const local_cpu_info& i)
{
	cc::write(os, "CPU package: {available threads: ${num}, uses SMT: ${bool}}",
		i.available_threads().size(), i.uses_smt());
	return os;
}

class global_cpu_info final
{
	std::vector<cpu_cache> m_caches{};
	cpu_version m_version{};
	uint8_t m_total_threads;
	uint8_t m_total_cores;
	uint8_t m_smt_id_bits;
	uint8_t m_core_id_bits;
	uint8_t m_pkg_id_bits;

	using cache_iterator       = decltype(m_caches.begin());
	using const_cache_iterator = decltype(m_caches.cbegin());
	using cache_range          = boost::iterator_range<cache_iterator>;
	using const_cache_range    = boost::iterator_range<const_cache_iterator>;
public:
	explicit global_cpu_info() noexcept {}

	cache_range caches() noexcept
	{ return {m_caches.begin(), m_caches.end()}; }

	const_cache_range caches() const noexcept
	{ return {m_caches.cbegin(), m_caches.cend()}; }

	void add(class cpu_cache& c) { m_caches.push_back(c); }

	uint8_t threads_per_core() const noexcept
	{ return m_total_threads / m_total_cores; }

	DEFINE_REF_GETTER_SETTER(global_cpu_info, version, m_version)
	DEFINE_COPY_GETTER_SETTER(global_cpu_info, total_threads, m_total_threads)
	DEFINE_COPY_GETTER_SETTER(global_cpu_info, total_cores, m_total_cores)
	DEFINE_COPY_GETTER_SETTER(global_cpu_info, smt_id_bits, m_smt_id_bits)
	DEFINE_COPY_GETTER_SETTER(global_cpu_info, core_id_bits, m_core_id_bits)
	DEFINE_COPY_GETTER_SETTER(global_cpu_info, package_id_bits, m_pkg_id_bits)
};

std::ostream& operator<<(std::ostream& os, const global_cpu_info& i)
{
	cc::write(os, "CPU package: {version: ${quote}, total cores: ${num}, "
		"total threads: ${num}}", i.version(), i.total_cores(),
		i.total_threads());
	return os;
}

/*
** TODO implement the following methods:
**   - get apic id of processor:
**     - thread_id(uint32_t, const global_cpu_info&)
**     - core_id(uint32_t, const global_cpu_info&)
**     - processor_id(uint32_t, const global_cpu_info&)
**   - get neighbor count of local thread info object (the only level for which
**   this makes sense is the core level, since the local processor info object
**   contains info about neighbors at the processor level)
**   - define function that can be used to compare two different local thread
**   info objects, given a global_thread_info object
*/

uint32_t core_id(
	const cpu_thread_info& thread,
	const global_cpu_info& info
) noexcept
{
	return thread.x2apic_id() >> info.smt_id_bits();
}

uint32_t package_id(
	const cpu_thread_info& thread,
	const global_cpu_info& info
) noexcept
{
	return thread.x2apic_id() >> (info.smt_id_bits() + info.core_id_bits());
}

class numa_node_info final
{
	local_cpu_info m_cpu_info{};
	uint32_t m_id{};
public:
	explicit numa_node_info() noexcept {}

	class local_cpu_info& cpu_info()
	noexcept { return m_cpu_info; }

	const class local_cpu_info& cpu_info()
	const noexcept { return m_cpu_info; }

	DEFINE_COPY_GETTER_SETTER(numa_node_info, id, m_id)
};

std::ostream& operator<<(std::ostream& os, const numa_node_info& i)
{
	cc::write(os, "NUMA node: ID $", i.id());
	return os;
}

/*
** TODO: Implement the following as *global functions*:
** - Obtaining the size of a NUMA node (total memory).
** - Allocation and deallocation.
** - Changing the allocation policy for the node.
*/

class system_info final
{
	global_cpu_info m_cpu_info{};
	std::vector<numa_node_info> m_node_info{};
	std::vector<cpu_thread_info> m_cpu_thread_info{};
	uint32_t m_total_nodes;

	using numa_node_iterator       = decltype(m_node_info.begin());
	using const_numa_node_iterator = decltype(m_node_info.cbegin());
	using numa_node_range          = boost::iterator_range<numa_node_iterator>;
	using const_numa_node_range    = boost::iterator_range<const_numa_node_iterator>;

	using cpu_thread_iterator       = decltype(m_cpu_thread_info.begin());
	using const_cpu_thread_iterator = decltype(m_cpu_thread_info.cbegin());
	using cpu_thread_range          = boost::iterator_range<cpu_thread_iterator>;
	using const_cpu_thread_range    = boost::iterator_range<const_cpu_thread_iterator>;
public:
	explicit system_info() noexcept {}

	size_t total_numa_nodes() const noexcept
	{ return m_total_nodes; }

	size_t total_cpu_threads() const noexcept
	{ return m_total_nodes * m_cpu_info.total_threads(); }

	system_info& total_numa_nodes(size_t n) noexcept
	{
		m_total_nodes = n;
		return *this;
	}

	system_info& available_numa_nodes(size_t n)
	{
		m_node_info.resize(n);
		return *this;
	}

	system_info& available_cpu_threads(size_t n)
	{
		m_cpu_thread_info.resize(n);
		return *this;
	}

	void add(const numa_node_info& n) { m_node_info.push_back(n); }
	void add(const cpu_thread_info& i) { m_cpu_thread_info.push_back(i); }

	numa_node_range available_numa_nodes() noexcept
	{ return {m_node_info.begin(), m_node_info.end()}; }

	const_numa_node_range available_numa_nodes() const noexcept
	{ return {m_node_info.cbegin(), m_node_info.cend()}; }

	cpu_thread_range available_cpu_threads() noexcept
	{ return {m_cpu_thread_info.begin(), m_cpu_thread_info.end()}; }

	const_cpu_thread_range available_cpu_threads() const noexcept
	{ return {m_cpu_thread_info.cbegin(), m_cpu_thread_info.cend()}; }

	global_cpu_info& cpu_info() noexcept
	{ return m_cpu_info; }

	const global_cpu_info& cpu_info() const noexcept
	{ return m_cpu_info; }
};

std::ostream& operator<<(std::ostream& os, const system_info& i)
{
	cc::write(os, "system info: {NUMA nodes available: $/$,  cpu threads available: $/$}",
		i.available_numa_nodes(), i.total_numa_nodes(),
		i.available_cpu_threads().size(), i.total_cpu_threads());
	return os;
}

}

#endif
