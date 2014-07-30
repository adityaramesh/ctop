/*
** File Name: processor_query_test.cpp
** Author:    Aditya Ramesh
** Date:      07/30/2014
** Contact:   _@adityaramesh.com
*/

#include <ccbase/format.hpp>
#include <ctop/processor_query.hpp>

int main()
{
	cc::println(ctop::processor_description().get());
}
