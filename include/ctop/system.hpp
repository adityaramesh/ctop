/*
** File Name: processor.hpp
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
	cc::write(os, "{{brand_string: \"$\", base_frequency: $ MHz, type: $, "
		"family: $, model: $, stepping: $}}", v.brand(),
		v.base_frequency(), v.type(), (int)v.family(), (int)v.model(),
		(int)v.stepping());
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
		cc::write(os, "\"Instruction\"");
		return os;
	case cache_type::data:
		cc::write(os, "\"Data\"");
		return os;
	case cache_type::unified:
		cc::write(os, "\"Unified\"");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

class cache
{
	uint8_t m_level{};
	uint8_t m_assoc{};
	cache_type m_type{};
	size_t m_size{};
	size_t m_line_size{};
public:
	explicit cache() noexcept {}

	DEFINE_COPY_GETTER_SETTER(cache, level, m_level)
	DEFINE_COPY_GETTER_SETTER(cache, associativity, m_assoc)
	DEFINE_COPY_GETTER_SETTER(cache, type, m_type)
	DEFINE_COPY_GETTER_SETTER(cache, size, m_size)
	DEFINE_COPY_GETTER_SETTER(cache, line_size, m_line_size)
};

std::ostream& operator<<(std::ostream& os, const cache& c)
{
	cc::write(os, "{{level: $, type: $, associativity: $, size: $, "
		"line_size: $}}", (int)c.level(), c.type(),
		(int)c.associativity(), (int)c.size(), (int)c.line_size());
	return os;
}

class core;
class processor_package;

class thread
{
	core* m_core{};
	uint32_t m_x2apic_id{};
public:
	explicit thread() noexcept {}

	class core& core() { return *m_core; }
	const class core& core() const { return *m_core; }

	DEFINE_COPY_GETTER_SETTER(thread, x2apic_id, m_x2apic_id)
};

std::ostream& operator<<(std::ostream& os, const thread& t)
{
	os << "{x2apic_id: " << std::hex << t.x2apic_id() << "}";
	return os;
}

class core
{
	std::vector<thread*> m_threads{};
	std::vector<cache*> m_caches{};
	processor_package* m_pkg{};
	uint32_t m_x2apic_id{};
public:
	explicit core() noexcept {}

	processor_package& package()
	{ return *m_pkg; }

	const processor_package& package() const
	{ return *m_pkg; }

	size_t threads() const { return m_threads.size(); }
	class thread& thread(size_t i) { return *m_threads[i]; }
	const class thread& thread(size_t i) const { return *m_threads[i]; }

	size_t caches() const { return m_caches.size(); }
	class cache& cache(size_t i) { return *m_caches[i]; }
	const class cache& cache(size_t i) const { return *m_caches[i]; }

	DEFINE_COPY_GETTER_SETTER(core, x2apic_id, m_x2apic_id)
};

std::ostream& operator<<(std::ostream& os, const core& c)
{
	os << "{x2apic_id: " << std::hex << c.x2apic_id() << ", ";

	os << "threads: [";
	if (c.threads() != 0) {
		for (auto i = 0; i != c.threads() - 1; ++i) {
			os << c.thread(i) << ", ";
		}
		os << c.thread(c.threads() - 1);
	}
	os << "], ";

	os << "caches: [";
	if (c.caches() != 0) {
		for (auto i = 0; i != c.caches() - 1; ++i) {
			os << c.cache(i) << ", ";
		}
		os << c.cache(c.caches() - 1);
	}
	os << "]}";
	return os;
}

class processor_package
{
	std::vector<core*> m_cores{};
	std::vector<cache*> m_caches{};
	vendor m_vendor{};
	processor_version m_version{};
	uint32_t m_x2apic_id{};
	uint8_t m_max_cores{};
	uint8_t m_max_threads{};
public:
	explicit processor_package() noexcept {}

	size_t cores() const { return m_cores.size(); }
	class core& core(size_t i) { return *m_cores[i]; }
	const class core& core(size_t i) const { return *m_cores[i]; }

	size_t caches() const { return m_caches.size(); }
	class cache& cache(size_t i) { return *m_caches[i]; }
	const class cache& cache(size_t i) const { return *m_caches[i]; }

	DEFINE_COPY_GETTER_SETTER(processor_package, vendor, m_vendor)
	DEFINE_REF_GETTER_SETTER(processor_package, version, m_version)
	DEFINE_REF_GETTER_SETTER(processor_package, x2apic_id, m_x2apic_id)
	DEFINE_COPY_GETTER_SETTER(processor_package, max_cores, m_max_cores)
	DEFINE_COPY_GETTER_SETTER(processor_package, max_threads, m_max_threads)
};

std::ostream& operator<<(std::ostream& os, const processor_package& p)
{
	cc::write(os, "{{vendor: $, version: $, x2apic_id: $, "
		"max_cores: $, max_threads: $, ", p.vendor(), p.version(),
		reinterpret_cast<void*>(p.x2apic_id()), (int)p.max_cores(),
		(int)p.max_threads());

	os << "cores: [";
	if (p.cores() != 0) {
		for (auto i = 0; i != p.cores() - 1; ++i) {
			os << p.core(i) << ", ";
		}
		os << p.core(p.cores() - 1);
	}
	os << "], ";

	os << "caches: [";
	if (p.caches() != 0) {
		for (auto i = 0; i != p.caches() - 1; ++i) {
			os << p.cache(i) << ", ";
		}
		os << p.cache(p.caches() - 1);
	}
	os << "]}";
	return os;
}

class system
{

};

}

#endif
