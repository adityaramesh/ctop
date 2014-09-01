/*
** File Name: thread_test.cpp
** Author:    Aditya Ramesh
** Date:      07/31/2014
** Contact:   _@adityaramesh.com
*/

#include <cmath>
#include <cstdint>
#include <iostream>
#include <system_error>

#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/mach_error.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>

#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <mach/i386/thread_status.h>

void check(kern_return_t err)
{
        if (err == KERN_SUCCESS) return;
	throw std::runtime_error{::mach_error_string(err)};
}

static void test()
{
	std::puts("hello");
}

int main()
{
        auto thread     = ::thread_t{};
        auto task       = ::mach_task_self();
        auto stack_size = 65536;
        auto stack      = (::vm_address_t)nullptr;
	check(::mach_vm_allocate(task, (::mach_vm_offset_t*)&stack, stack_size,
		VM_FLAGS_ANYWHERE));

        auto state     = ::x86_thread_state64_t{};
        auto count     = ::mach_msg_type_number_t{x86_THREAD_STATE64_COUNT};
        auto stack_ptr = stack + stack_size / 2 - 8;
        state.__rip = (uintptr_t)test;
        state.__rsp = (uintptr_t)stack_ptr;
        state.__rbp = (uintptr_t)stack_ptr;

	check(::thread_create_running(task, x86_THREAD_STATE64,
		(thread_state_t)&state, x86_THREAD_STATE64_COUNT, &thread));
        //::sleep(1);
	auto c = 0;
	for (auto i = 0ull; i != 100000000000; ++i) {
		c += std::sin((float)i) * std::exp(1./i);
	}
	std::cout << c << std::endl;
        std::cout << "Done." << std::endl;
	// TODO free thread reference
}
