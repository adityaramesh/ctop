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
	auto info = ctop::system_query().get();

	// Print global information.
	cc::println(info);
	cc::println(info.cpu_info());
	for (auto i = size_t{0}; i != info.cpu_info().caches(); ++i) {
		cc::println(info.cpu_info().cache(i));
	}

	// Print information local to each NUMA node.
	for (auto i = size_t{0}; i != info.available_numa_nodes(); ++i) {
		auto& node = info.numa_node_info(i);
		auto& cpu = node.cpu_info();
		cc::println(node);
		cc::println(cpu);
		for (auto j = size_t{0}; j != cpu.available_threads(); ++j) {
			cc::println(cpu.thread_info(j));
		}
	}
}
