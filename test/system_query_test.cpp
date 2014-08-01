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
	cc::println(ctop::system_query().get());
}
