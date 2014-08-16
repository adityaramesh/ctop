/*
** File Name: system_query_test.cpp
** Author:    Aditya Ramesh
** Date:      07/30/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/format.hpp>
#include <ctop/system_query.hpp>

int main()
{
	auto info = *ctop::system_query();

	// Print global information.
	cc::println(info);
	cc::println(info.cpu_info());
	for (const auto& cache : info.cpu_info().caches()) {
		cc::println(cache);
	}

	// Print information local to each NUMA node.
	for (const auto& node : info.available_numa_nodes()) {
		auto& cpu = node.cpu_info();
		cc::println(node);
		cc::println(cpu);
		for (const auto& thread : cpu.available_threads()) {
			cc::println(thread);
		}
	}
}
