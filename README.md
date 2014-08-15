<!--
  ** File Name: README.md
  ** Author:    Aditya Ramesh
  ** Date:      07/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Introduction

`ctop` is a C++ library that allows you to analyze system topology information.
Much of this information is obtained from the `CPUID` instruction. This
information can be very useful to have in multithreaded applications.

`ctop` uses stringent error checking to verify that assumptions made about the
system actually hold. Careful attention has been given to ensure that when
something fails, a clear error message is produced that explains where things
went wrong.

# Assumptions

The following assumptions are made about the system on which this code is run:
  - The system is running the Linux kernel.
  - All CPUs on the system are of the same type.
  - The CPU microarchitecture is Intel Nehalem or later. (Support for AMD
  processors is planned for the future.)
  - Each NUMA node on the system contains exactly one CPU.

Where possible, the library statically checks to ensure that these assumptions
hold. Otherwise, exceptions are thrown during runtime.

# TODO

- Support the following auxiliary (PCIe) devices:
  - GPUs
  - MICs

- Support the following NUMA devices:
  - MICs

- Assume that there is always an "SMT" level.
- Do not assume that the hierarchical levels are reported in order with
increasing values of ECX -- fix this issue in the code.
- Assume that if there is only one logical CPU per core, then HTT is disabled.
- Even cores and threads that are not accessible to the process should be added
to the lists in the `processor_package` class.

- Strategy overview
  - Instantiate system object [DONE]
  - Initialize global CPU info
    - Caches [DONE]
    - APIC sub-id mask widths [DONE]
    - Processor counts using CPUID leaf 11 (but do not store APIC IDs yet) [DONE]
  - Initialize NUMA info
    - Get total NUMA nodes [DONE]
    - Get available NUMA nodes [DONE]
    - Get available CPU threads [DONE]
    - For each NUMA node
      - For each thread associated with the NUMA node
        - Get the pointer to the back of the cpu_thread_info vector
        - Add a new cpu_thread_info object to `system` with the APIC ID and OS
	ID
        - Increment a counter
	- Set the thread info pointer and length fields of the associated
	local_cpu_info object.

***************************
- XXX: add support for many CPU packages in the same NUMA node -- nehalem and
future architectures may be like this
***************************

# Scratch

#if PLATFORM_COMPILER == PLATFORM_COMPILER_GCC || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_CLANG || \
    PLATFORM_COMPILER == PLATFORM_COMPILER_ICC

CC_CONST CC_ALWAYS_INLINE uint32_t 
next_power_of_two(uint32_t x)
{
	return 1 << (32 - __builtin_clz(x - 1));
}

#else
	#error "Unsupported compiler."
#endif

- Expected:
  - alternative to `return true;` for expected<void> -- cc::no_error
  - unit tests
  - rewrite documentation
    - mention lvalue/rvalue overloads for compatibility with move semantics
    - mention CC_RETURN_ON_ERROR

  - After merging expected changes to master:
  - Convert library to CC_NO_DEBUG and CC_ASSERT
  - Add final declarations where appropriate.
  - Add info about CC_NO_DEBUG to documentation.
