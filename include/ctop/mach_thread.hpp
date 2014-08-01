/*
** File Name: mach_thread.hpp
** Author:    Aditya Ramesh
** Date:      07/30/2014
** Contact:   _@adityaramesh.com
*/

#ifndef ZE54701F2_215B_4C86_B695_578B8906C616
#define ZE54701F2_215B_4C86_B695_578B8906C616

#include <system_error>
#include <ccbase/error.hpp>
#include <ccbase/platform.hpp>

#if PLATFORM_KERNEL == PLATFORM_KERNEL_XNU
	#include <unistd.h>
	#include <mach/mach_init.h>
	#include <mach/mach_types.h>
	#include <mach/task.h>
	#include <mach/thread_act.h>
	#include <mach/thread_policy.h>
	#include <mach/i386/thread_status.h>
#elif
	#error "Unsupported kernel."
#endif

namespace ctop {

std::system_error to_system_error(kern_return_t err)
{
	auto code = std::error_code{err, std::system_category()};
	switch (err) {
	case KERN_FAILURE:
		return std::system_error{code, "Failure"};
	case KERN_INVALID_ARGUMENT:
		return std::system_error{code, "Invalid argument"};
	default:
		return std::system_error{code, "Unknown system error"};
	}
}

cc::expected<void>
thread_create(task_t parent_task, thread_t* child_thread)
{
	auto r = ::thread_create(parent_task, child_thread);
	if (r == KERN_SUCCESS) {
		return true;
	}
	else {
		return to_system_error(r);
	}
}

cc::expected<void>
thread_get_state(
	thread_t target_thread,
	int flavor,
	thread_state_t old_state,
	mach_msg_type_number_t* old_state_cnt
)
{
	auto r = ::thread_get_state(target_thread, flavor, old_state, old_state_cnt);
	if (r == KERN_SUCCESS) {
		return true;
	}
	else {
		return to_system_error(r);
	}
}

cc::expected<void>
thread_set_state(
	thread_t target_thread,
	int flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_state_cnt
)
{
	auto r = ::thread_set_state(target_thread, flavor, new_state, new_state_cnt);
	if (r == KERN_SUCCESS) {
		return true;
	}
	else {
		return to_system_error(r);
	}
}

cc::expected<void>
thread_resume(thread_t target_thread)
{
	auto r = ::thread_resume(target_thread);
	if (r == KERN_SUCCESS) {
		return true;
	}
	else {
		return to_system_error(r);
	}
}

cc::expected<void>
thread_suspend(thread_t target_thread)
{
	auto r = ::thread_suspend(target_thread);
	if (r == KERN_SUCCESS) {
		return true;
	}
	else {
		return to_system_error(r);
	}
}

}

#endif
