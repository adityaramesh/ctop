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

- Add support for AMD.
- Test on fatalii (for AMD support).

- Support the following auxiliary (PCIe) devices:
  - GPUs
  - MICs

- Support the following NUMA devices:
  - MICs

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
