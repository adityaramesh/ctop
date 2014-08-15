/*
** File Name:	cpuid_error.hpp
** Author:	Aditya Ramesh
** Date:	08/12/2014
** Contact:	_@adityaramesh.com
*/

#ifndef ZAF2CA779_99A2_41ED_9C7C_33677EDCF1EF
#define ZAF2CA779_99A2_41ED_9C7C_33677EDCF1EF

#include <stdexcept>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>
#include <ctop/cpuid_leaf.hpp>
#include <boost/utility/string_ref.hpp>

namespace ctop {

class cpuid_unsupported_error final : public std::runtime_error
{
public:
	explicit cpuid_unsupported_error() : 
	std::runtime_error{"CPUID unsupported."} {}
};

class cpuid_error final : public std::exception
{
	uint32_t m_leaf;
	std::string m_msg{};
public:
	explicit cpuid_error(uint32_t leaf, const boost::string_ref& msg)
	noexcept : m_leaf{leaf}
	{
		m_msg = cc::format("Error using CPUID leaf ${quote} (${hex, base}): $.",
			cpuid_leaf::to_string(leaf), leaf, msg);
	}

	const char* what() const noexcept override
	{ return m_msg.c_str(); }

	DEFINE_COPY_GETTER_SETTER(cpuid_error, leaf, m_leaf)
};

}

#endif
