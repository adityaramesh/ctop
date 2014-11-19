/*
** File Name: pthread_test.cpp
** Author:    Aditya Ramesh
** Date:      11/18/2014
** Contact:   _@adityaramesh.com
**
** Even if we create the threads in suspended states, and assign each thread a
** distinct policy tag before starting the threads, the scheduler may still run
** two threads with different policy tags on the same processor. So it seems
** that this is the best we can do for OS X.
*/

#include <pthread.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#include <numeric>
#include <cstdint>
#include <iostream>
#include <vector>
#include <system_error>
#include <ctop/cpuid.hpp>
#include <ccbase/error.hpp>
#include <ccbase/format.hpp>

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

void* test(void* arg)
{
	const auto _ = std::ignore;
	auto r4 = uint32_t{};
	std::tie(_, _, _, r4) = ctop::cpuid(11, 0);
	std::cout << "My x2APIC ID: " << std::hex << r4 << "." << std::endl;
	return nullptr;
}

int main()
{
	auto nprocs = *get_integer_property("hw.logicalcpu");
	auto threads = std::vector<::pthread_t>(nprocs);
	auto tags = std::vector<int>(nprocs);
	auto cnt = THREAD_AFFINITY_POLICY_COUNT;
	std::iota(tags.begin(), tags.end(), 0);

	for (auto i = 0; i != nprocs; ++i) {
		::pthread_create_suspended_np(&threads[i], nullptr, test, &tags[i]);
		auto mth = ::pthread_mach_thread_np(threads[i]);
		auto pol = ::thread_affinity_policy{i};
		::thread_policy_set(
			::pthread_mach_thread_np(threads[i]),
			THREAD_AFFINITY_POLICY,
			(::thread_policy_t)&pol, 1
		);
	}

	for (auto i = 0; i != nprocs; ++i) {
		::thread_resume(::pthread_mach_thread_np(threads[i]));
	}

	for (auto i = 0; i != nprocs; ++i) {
		::pthread_join(threads[i], nullptr);
	}
}
