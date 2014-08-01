/*
** File Name: system_topology.cpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
**
** This program outputs the following system topology information on OS X:
**   - Number of processor packages present on machine.
**   - Number of physical CPUs available in current power mode.
**   - Maximum number of available physical CPUs.
**   - Same two stats for logical CPUs.
**   - Number of CPU cache levels.
**   - Size of each CPU cache.
**   - Sharing attributes of each cache level.
*/

#include <vector>
#include <system_error>
#include <ccbase/error.hpp>
#include <ccbase/format.hpp>
#include <ccbase/platform.hpp>
#include <boost/range/algorithm.hpp>

#if PLATFORM_KERNEL != PLATFORM_KERNEL_XNU
	#error "Unsupported kernel."
#endif

#include <sys/types.h>
#include <sys/sysctl.h>

cc::expected<uint32_t>
get_integer_property(const char* name)
{
	auto buf = uint32_t{};
	auto size = sizeof(buf);
	if (::sysctlbyname(name, &buf, &size, nullptr, 0) != 0) {
		return std::system_error{errno, std::system_category()};
	}
	return buf;
}

cc::expected<std::vector<uint64_t>>
get_array_property(const char* name)
{
	auto size = size_t{};
	if (::sysctlbyname(name, nullptr, &size, nullptr, 0) != 0) {
		return std::system_error{errno, std::system_category()};
	}

	assert(size > 0);
	auto vec = std::vector<uint64_t>(size / sizeof(uint64_t));
	if (::sysctlbyname(name, vec.data(), &size, nullptr, 0) != 0) {
		return std::system_error{errno, std::system_category()};
	}
	return vec;
}

int main()
{
	auto pkgs = get_integer_property("hw.packages").get();
	cc::println("Number of CPU packages: $.", pkgs);

	auto phys = get_integer_property("hw.physicalcpu").get();
	auto phys_max = get_integer_property("hw.physicalcpu_max").get();
	cc::println("Number of physical CPUs available: $/$.", phys, phys_max);

	auto log = get_integer_property("hw.logicalcpu").get();
	auto log_max = get_integer_property("hw.logicalcpu_max").get();
	cc::println("Number of logical CPUs available: $/$.", log, log_max);

	auto cache_config = get_array_property("hw.cacheconfig").move();
	auto cache_sizes = get_array_property("hw.cachesize").move();

	auto end = boost::find_if(cache_config, [](auto x) { return x == 0; });
	assert(end != cache_config.begin());
	assert(end != cache_config.end());
	auto cache_levels = end - cache_config.begin() - 1;

	cc::println("Number of cache levels: $.", cache_levels);
}
