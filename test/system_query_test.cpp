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
	auto pkg = ctop::system_query().get();
	cc::println(pkg);
	for (auto i = 0; i != pkg.caches(); ++i) {
		cc::println(pkg.cache(i));
	}
}
