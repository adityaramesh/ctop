/*
** File Name: thread_test.cpp
** Author:    Aditya Ramesh
** Date:      07/31/2014
** Contact:   _@adityaramesh.com
*/

#include <cstdint>
#include <iostream>
#include <system_error>
 
#include <unistd.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <mach/i386/thread_status.h>
 
void check(kern_return_t err)
{
        if (err == KERN_SUCCESS) {
                return;
        }
 
        auto code = std::error_code{err, std::system_category()};
        switch (err) {
        case KERN_FAILURE:
                throw std::system_error{code, "failure"};
        case KERN_INVALID_ARGUMENT:
                throw std::system_error{code, "invalid argument"};
        default:
                throw std::system_error{code, "unknown error"};
        }
}
 
void test_function()
{
        std::cout << "Hello from thread." << std::endl;
	std::cout << std::flush;
	exit(0);
}
 
int main()
{
}
