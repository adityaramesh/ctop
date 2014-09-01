/*
** File Name: thread_test.cpp
** Author:    Aditya Ramesh
** Date:      07/30/2014
** Contact:   _@adityaramesh.com
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

std::atomic<bool> barrier{false};

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
	auto mth = ::pthread_mach_thread_np(::pthread_self());
	auto cnt = THREAD_AFFINITY_POLICY_COUNT;
	auto pol = ::thread_affinity_policy{};
	pol.affinity_tag = *(int*)arg;

	if (::thread_policy_set(mth, THREAD_AFFINITY_POLICY,
		(::thread_policy_t)&pol, cnt) != KERN_SUCCESS)
	{
		cc::println("Failed to set thread policy.");
		return nullptr;
	}

	std::cout << "Waiting..." << std::endl;
	while (!barrier) {}
	::pthread_yield_np();

	uint32_t r1, r2, r3, r4;
	std::tie(r1, r2, r3, r4) = ctop::cpuid(11, 0);
	std::cout << "My x2APIC ID: " << std::hex << r4 << "." << std::endl;
	return nullptr;
}

int main()
{
	auto nprocs = *get_integer_property("hw.logicalcpu");
	auto threads = std::vector<::pthread_t>(nprocs);
	auto tags = std::vector<int>(nprocs);
	std::iota(tags.begin(), tags.end(), 0);

	for (auto i = 0; i != nprocs; ++i) {
		::pthread_create(&threads[i], nullptr, test, &tags[i]);
	}
	barrier = true;

	for (auto i = 0; i != nprocs; ++i) {
		::pthread_join(threads[i], nullptr);
	}
}
