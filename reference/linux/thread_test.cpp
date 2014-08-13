/*
** File Name:	thread_test.cpp
** Author:	Aditya Ramesh
** Date:	07/31/2014
** Contact:	_@adityaramesh.com
*/

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <cstdint>
#include <tuple>
#include <system_error>

#include <ccbase/format.hpp>
#include <ctop/cpuid.hpp>

// For `sysconf`.
#include <unistd.h>
// For `CPU_SET`.
#include <sched.h>

void check(int r)
{
	if (r == 0) return;
	throw std::system_error{r, std::system_category()};
}

void print_cpu_info()
{
	uint32_t r1, r2, r3, r4;
	auto level = 0;

	for (;;) {
		std::tie(r1, r2, r3, r4) = ctop::cpuid(11, level);
		++level;

		auto level_type = (r3 >> 8) & 0xFF;
		if (level_type == 0) {
			break;
		}

		auto shift = r1 & 0x1F;
		auto count = r2 & 0xFFFF;
		auto level_id = r4 >> shift;
		cc::println("level: $, count: $, level type: $, "
			"level id: $, apic id: $.", level - 1, count,
			level_type, level_id, r4);
	}

	std::tie(r1, r2, r3, r4) = ctop::cpuid(1);
	cc::println((r2 & 0x00FF0000) >> 16);
}

int main()
{
	if (ctop::max_cpuid_leaf().get() < 11) {
		cc::errln("Unsupported CPU.");
		return EXIT_FAILURE;
	}

	auto nprocs = ::sysconf(_SC_NPROCESSORS_ONLN);
	auto set = ::cpu_set_t{};
	cc::println("Processors online: $.", nprocs);

	for (auto i = 0; i != nprocs; ++i) {
		CPU_SET(i, &set);
		check(::sched_setaffinity(0, sizeof(::cpu_set_t), &set));
		CPU_CLR(i, &set);
		print_cpu_info();
	}
}
