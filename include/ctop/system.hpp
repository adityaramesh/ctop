/*
** File Name: system.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z81EA1B11_A653_467B_BFC2_F5F532D5D3F8
#define Z81EA1B11_A653_467B_BFC2_F5F532D5D3F8

#include <array>
#include <ostream>
#include <vector>
#include <boost/utility/string_ref.hpp>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>
#include <ctop/vendor.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_LINUX
	#include <numa.h>
#else
	#error "Unsupported platform kernel."
#endif

namespace ctop {

enum class processor_type
{
	original_oem    = 0x0,
	intel_overdrive = 0x1,
	dual_processor  = 0x2,
	reserved        = 0x3,
};

std::ostream& operator<<(std::ostream& os, const processor_type& t)
{
	switch (t) {
	case processor_type::original_oem:
		cc::write(os, "\"Original OEM\"");
		return os;
	case processor_type::intel_overdrive:
		cc::write(os, "\"Intel OverDrive\"");
		return os;
	case processor_type::dual_processor:
		cc::write(os, "\"Dual Processor\"");
		return os;
	case processor_type::reserved:
		cc::write(os, "\"Reserved\"");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

class processor_version
{
	static constexpr auto brand_length = 48;

	std::array<char, brand_length> m_brand{};
	double m_base_freq_mhz{};
	uint8_t m_brand_off{};
	uint8_t m_family{};
	uint8_t m_model{};
	uint8_t m_stepping{};
	processor_type m_type{};
public:
	explicit processor_version() noexcept {}

	boost::string_ref brand()
	{
		return {m_brand.data() + m_brand_off, (size_t)(brand_length - m_brand_off)};
	}

	const boost::string_ref brand() const
	{
		return {m_brand.data() + m_brand_off, (size_t)(brand_length - m_brand_off)};
	}

	DEFINE_COPY_GETTER_SETTER(processor_version, base_frequency, m_base_freq_mhz)
	DEFINE_COPY_GETTER_SETTER(processor_version, brand_offset, m_brand_off)
	DEFINE_COPY_GETTER_SETTER(processor_version, family, m_family)
	DEFINE_COPY_GETTER_SETTER(processor_version, model, m_model)
	DEFINE_COPY_GETTER_SETTER(processor_version, stepping, m_stepping)
	DEFINE_COPY_GETTER_SETTER(processor_version, type, m_type)
};

std::ostream& operator<<(std::ostream& os, const processor_version& v)
{
	cc::write(os, v.brand());
	return os;
}

enum class cache_type
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

class cache
{
	bool m_self_init{};
	bool m_fully_assoc{};
	bool m_inv_propagation{};
	bool m_inclusive{};
	bool m_direct{};

	cache_type m_type{};
	uint8_t m_level{};
	uint32_t m_sharing_threads{};
	uint32_t m_size{};
	uint32_t m_sets{};
	uint32_t m_line_size{};
	uint32_t m_line_partitions{};
	uint32_t m_assoc{};
public:
	explicit cache() noexcept {}

	DEFINE_COPY_GETTER_SETTER(cache, is_self_initializing, m_self_init)
	DEFINE_COPY_GETTER_SETTER(cache, is_fully_associative, m_fully_assoc)
	DEFINE_COPY_GETTER_SETTER(cache, has_invalidate_propagation, m_inv_propagation)
	DEFINE_COPY_GETTER_SETTER(cache, is_inclusive, m_inv_propagation)
	DEFINE_COPY_GETTER_SETTER(cache, is_direct_mapped, m_direct)

	DEFINE_COPY_GETTER_SETTER(cache, type, m_type)
	DEFINE_COPY_GETTER_SETTER(cache, level, m_level)
	DEFINE_COPY_GETTER_SETTER(cache, sharing_threads, m_sharing_threads)
	DEFINE_COPY_GETTER_SETTER(cache, size, m_size)
	DEFINE_COPY_GETTER_SETTER(cache, sets, m_sets)
	DEFINE_COPY_GETTER_SETTER(cache, line_size, m_line_size)
	DEFINE_COPY_GETTER_SETTER(cache, line_partitions, m_line_partitions)
	DEFINE_COPY_GETTER_SETTER(cache, associativity, m_assoc)
};

std::ostream& operator<<(std::ostream& os, const cache& c)
{
	cc::write(os, "L${num} $ cache (${data})", c.level(), c.type(), c.size());
	return os;
}

class core;
class processor_package;

/*
** Every hardware execution resource is mapped to a thread in the virtual
** hierarchy, even if the processor does not support SMT. Therefore, in virtual
** hierarchy, each core of a processor contains at least one thread.
*/
class thread
{
	core* m_core{};

	/*
	** This information is only available if the process is able to run on
	** the associated thread.
	*/
	boost::optional<uint32_t> m_x2apic_id{};
	boost::optional<uint32_t> m_os_id{};
public:
	explicit thread() noexcept {}

	class core& core() { return *m_core; }
	const class core& core() const { return *m_core; }

	DEFINE_COPY_GETTER_SETTER(thread, x2apic_id, m_x2apic_id)
	DEFINE_COPY_GETTER_SETTER(thread, os_id, m_os_id)
};

std::ostream& operator<<(std::ostream& os, const thread& t)
{
	cc::write(os, "thread: {x2APIC ID: $, OS ID: $}",
		t.x2apic_id(), t.os_id());
	return os;
}

class core
{
	std::vector<thread*> m_threads{};
	std::vector<cache*> m_caches{};
	processor_package* m_pkg{};

	/*
	** This information is only available if the process is able to run on
	** the associated core.
	*/
	boost::optional<uint32_t> m_x2apic_id{};
	boost::optional<uint8_t> m_avail_threads{};
public:
	explicit core() noexcept {}

	processor_package& package()
	{ return *m_pkg; }

	const processor_package& package() const
	{ return *m_pkg; }

	size_t total_threads() const { return m_threads.size(); }
	class thread& thread(size_t i) { return *m_threads[i]; }
	const class thread& thread(size_t i) const { return *m_threads[i]; }

	size_t caches() const { return m_caches.size(); }
	class cache& cache(size_t i) { return *m_caches[i]; }
	const class cache& cache(size_t i) const { return *m_caches[i]; }

	void add(class thread& t) { m_threads.push_back(&t); }
	void add(class cache& c) { m_caches.push_back(&c); }

	DEFINE_COPY_GETTER_SETTER(core, x2apic_id, m_x2apic_id)
	DEFINE_COPY_GETTER_SETTER(core, available_threads, m_avail_threads)
};

std::ostream& operator<<(std::ostream& os, const core& c)
{
	cc::write(os, "core: {x2APIC ID: $, available threads: $/$}",
		c.x2apic_id(), c.available_threads(), c.total_threads());
	return os;
}

class processor_package
{
	std::vector<thread> m_threads{};
	std::vector<core> m_cores{};
	std::vector<cache> m_caches{};
	vendor m_vendor{};
	processor_version m_version{};

	/*
	** This information is only available if the process is able to run on
	** the associated processor.
	*/
	boost::optional<uint32_t> m_x2apic_id{};
	boost::optional<uint8_t> m_avail_cores{};
	boost::optional<uint8_t> m_avail_threads{};
	boost::optional<bool> m_uses_smt{};
public:
	explicit processor_package() noexcept {}

	size_t total_threads() const { return m_threads.size(); }
	class thread& thread(size_t i) { return m_threads[i]; }
	const class thread& thread(size_t i) const { return m_threads[i]; }

	size_t total_cores() const { return m_cores.size(); }
	class core& core(size_t i) { return m_cores[i]; }
	const class core& core(size_t i) const { return m_cores[i]; }

	size_t caches() const { return m_caches.size(); }
	class cache& cache(size_t i) { return m_caches[i]; }
	const class cache& cache(size_t i) const { return m_caches[i]; }

	void add(class thread& t) { m_threads.push_back(t); }
	void add(class core& c) { m_cores.push_back(c); }
	void add(class cache& c) { m_caches.push_back(c); }

	DEFINE_COPY_GETTER_SETTER(processor_package, vendor, m_vendor)
	DEFINE_REF_GETTER_SETTER(processor_package, version, m_version)
	DEFINE_REF_GETTER_SETTER(processor_package, x2apic_id, m_x2apic_id)
	DEFINE_COPY_GETTER_SETTER(processor_package, uses_smt, m_uses_smt)
	DEFINE_COPY_GETTER_SETTER(processor_package, available_cores, m_avail_cores)
	DEFINE_COPY_GETTER_SETTER(processor_package, available_threads, m_avail_threads)
};

std::ostream& operator<<(std::ostream& os, const processor_package& p)
{
	cc::write(os, "processor: {version: ${quote}, x2APIC ID: $,
		available cores: $/$}", p.version(), p.x2apic_id(),
		p.available_cores(), p.total_cores());
	return os;
}

/*
** For now, we assume that a NUMA node can only contain a single processor
** package.
*/
class numa_node
{
	processor_package m_pkg{};
	int m_id{};
	bool m_is_avail{};
public:
	explicit numa_node(
		const processor_package& pkg,
		bool is_accessible
	) noexcept : m_pkg{pkg}, m_is_avail{is_avail}
	{
		auto tid = pkg.core(0).thread(0).os_id();
		m_id = ::numa_node_of_cpu(tid);
		if (m_id == -1) {
			throw std::system_error{errno, std::system_category(),
				"failed to get NUMA node of CPU"};
		}
	}

	/*
	** TODO: Implement the following as *global functions*:
	** - Obtaining the size of a NUMA node (total memory).
	** - Allocation and deallocation.
	** - Changing the allocation policy for the node.
	*/

	bool is_available() const noexcept
	{ return m_is_avail; }

	class processor_package& processor_package()
	noexcept { return m_pkg; }

	const class processor_package& processor_package()
	const noexcept { return m_pkg; }
};

class system
{
	std::vector<numa_node> m_nodes{};
	int m_avail_nodes{};
public:
	explicit system() noexcept {}

	size_t total_numa_nodes() const { return m_nodes.size(); }
	class numa_node&(size_t i) { return m_nodes[i]; }
	const class numa_node&(size_t i) const { return m_nodes[i]; }
	void add(const numa_node& pkg) { m_nodes.push_back(p); }

	DEFINE_COPY_GETTER_SETTER(system, available_numa_nodes, m_avail_nodes)
};

}

#endif
