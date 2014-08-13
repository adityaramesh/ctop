<!--
  ** File Name: README.md
  ** Author:    Aditya Ramesh
  ** Date:      07/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Introduction

`ctop` is a C++ library that allows you to analyze system topology information.
Much of this information is obtained from the `CPUID` instruction. This
information is useful to have in multithreaded applications.

# Assumptions

The following assumptions are made about the system on which this code is run:
  - The system is running the Linux kernel.
  - All processors on the system are of the same type.
  - The processor microarchitecture is Intel Nehalem or later. (Support for AMD
  processors is planned for the future.)
  - Each NUMA node on the system contains exactly one processor socket.

Where possible, the library statically checks to ensure that these assumptions
hold. Otherwise, exceptions are thrown during runtime.

# TODO

- Use the method described here using CPUID leaf 11 to detect whether
hyperthreading is enabled:
http://stackoverflow.com/questions/2901694/programatically-detect-number-of-physical-processors-cores-or-if-hyper-threading

- Assume that there is always an "SMT" level.
- Do not assume that the hierarchical levels are reported in order with
increasing values of ECX -- fix this issue in the code.
- Assume that if there is only one logical CPU per core, then HTT is disabled.
- Even cores and threads that are not accessible to the process should be added
to the lists in the `processor_package` class.

- Strategy overview
  - Get total number of NUMA nodes
  - Get available count of NUMA nodes.
  - For each NUMA node, enabled or not, add a NUMA node to the system object.
  - For each available NUMA node
  - Global processor info
    - For each processor on the system

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

