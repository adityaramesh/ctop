/*
** File Name: system.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z008D83F8_D0FA_4815_96DE_FD47C220EB95
#define Z008D83F8_D0FA_4815_96DE_FD47C220EB95

#if PLATFORM_COMPILER == PLATFORM_COMPILER_GCC || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_CLANG
	#include <cpuid.h>
#else
	#error "Unsupported compiler."
#endif

#include <cstdint>
#include <tuple>
#include <boost/optional.hpp>

namespace ctop {

/*
** Returns the index corresponding to the largest supported CPUID leaf.
*/
boost::optional<uint32_t>
max_cpuid_leaf()
{
	auto r = __get_cpuid_max(0, nullptr);
	if (r == 0) {
		return boost::none;
	}
	return r;
}

/*
** Returns a four-tuple containing the contents of EAX, EBX, ECX, and EDX after
** executing the CPUID instruction with the given leaf index.
*/
auto get_cpuid(uint32_t leaf)
{
	uint32_t r1, r2, r3, r4;
	__cpuid(leaf, r1, r2, r3, r4);
	return std::make_tuple(r1, r2, r3, r4);
}

}

#endif
