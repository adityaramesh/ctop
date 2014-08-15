/*
** File Name:	numa_error.hpp
** Author:	Aditya Ramesh
** Date:	08/14/2014
** Contact:	_@adityaramesh.com
*/

#ifndef Z7D38AAFC_9F60_4938_8996_3BEC2DD6940C
#define Z7D38AAFC_9F60_4938_8996_3BEC2DD6940C

#include <exception>
#include <ccbase/format.hpp>
#include <ccbase/utility.hpp>

#include <boost/optional.hpp>
#include <boost/utility/string_ref.hpp>

namespace ctop {

class numa_error final : public std::exception
{
	boost::optional<uint32_t> m_node{};
	std::string m_msg{};
public:
	explicit numa_error(const std::string& msg)
	noexcept : m_msg{msg}
	{
		m_msg = cc::format("NUMA error: $.", msg);
	}

	explicit numa_error(uint32_t node, const boost::string_ref& msg)
	noexcept : m_node{node}
	{
		m_msg = cc::format("NUMA error (node $): $.", node, msg);
	}

	const char* what() const noexcept override
	{ return m_msg.c_str(); }

	DEFINE_COPY_GETTER_SETTER(numa_error, node, m_node)
};

}

#endif
