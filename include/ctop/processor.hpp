/*
** File Name: processor.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z81EA1B11_A653_467B_BFC2_F5F532D5D3F8
#define Z81EA1B11_A653_467B_BFC2_F5F532D5D3F8

#include <array>
#include <ratio>
#include <ostream>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>
#include <ctop/vendor.hpp>

namespace ctop {

enum class processor_type
{
	original_oem,
	intel_overdrive,
	dual_processor,
	reserved,
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
	std::array<int8_t, 48> m_brand{};
	double m_base_freq_mhz{};
	uint8_t m_family{};
	uint8_t m_model{};
	uint8_t m_stepping{};
	processor_type m_type{};
public:
	explicit processor_version() noexcept {}

	int8_t* brand() { return m_brand.data(); }
	const int8_t* brand() const { return m_brand.data(); }

	DEFINE_COPY_GETTER_SETTER(processor_version, base_frequency, m_base_freq_mhz)
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

class processor_package
{
	vendor m_vendor{};
	processor_version m_version{};
	size_t m_cache_line_size{};
	uint8_t m_phys_procs{};
	uint8_t m_log_procs{};
public:
	explicit processor_package() noexcept {}

	DEFINE_COPY_GETTER_SETTER(processor_package, vendor, m_vendor)
	DEFINE_REF_GETTER_SETTER(processor_package, version, m_version)
	DEFINE_REF_GETTER_SETTER(processor_package, cache_line_size, m_cache_line_size)
	DEFINE_COPY_GETTER_SETTER(processor_package, physical_processors, m_phys_procs)
	DEFINE_COPY_GETTER_SETTER(processor_package, logical_processors, m_log_procs)
};

std::ostream& operator<<(std::ostream& os, const processor_package& p)
{
	cc::write(os, "{{vendor: $, version: $, physical_processors: $, "
		"logical_processors: $}}", p.vendor(), p.version(),
		(int)p.physical_processors(), (int)p.logical_processors());
	return os;
}

}

#endif
