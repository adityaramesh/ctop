/*
** File Name: system.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z008D83F8_D0FA_4815_96DE_FD47C220EB95
#define Z008D83F8_D0FA_4815_96DE_FD47C220EB95

#if PLATFORM_COMPILER == PLATFORM_COMPILER_GCC   || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_CLANG || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_ICC
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
**
** IMPORTANT NOTE: The use of the "volatile" keyword is **crucial** here,
** because the compiler treats the ASM instrinsic as a constant function, even
** if fences are placed around the calling site. This means that the compiler
** can hoist `cpuid` outside of a loop in which the thread is scheduled to a new
** processor at each iteration.
*/
auto cpuid(uint32_t leaf) ->
std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>
{
	uint32_t r1, r2, r3, r4;
	asm volatile("cpuid"
		: "=a" (r1), "=b" (r2), "=c" (r3), "=d" (r4)
		: "a" (leaf)
	);
	return std::make_tuple(r1, r2, r3, r4);
}

/*
** This function does the same thing as the unary `cpuid` function, but it also
** loads the given value into ECX before executing CPUID. This is useful for the
** CPUID leaves that take two arguments as input.
**
** IMPORTANT NOTE: The use of the "volatile" keyword is **crucial** here,
** because the compiler treats the ASM instrinsic as a constant function, even
** if fences are placed around the calling site. This means that the compiler
** can hoist `cpuid` outside of a loop in which the thread is scheduled to a new
** processor at each iteration.
*/
auto cpuid(uint32_t leaf, uint32_t arg) ->
std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>
{
	uint32_t r1, r2, r3, r4;
	asm volatile("cpuid"
		: "=a" (r1), "=b" (r2), "=c" (r3), "=d" (r4)
		: "a" (leaf), "c" (arg)
	);
	return std::make_tuple(r1, r2, r3, r4);
}

}

#endif
