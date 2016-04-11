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

# Agenda

- Change line width to 100.
- Throw instead of using `expected`.
- Minimize dependencies to ccbase and boost.

- For each core, add a mapping to the corresponding bit in CPU_SET.
- Finish TODOs in the files.

# Assumptions

The following assumptions are made about the system on which this code is run:
  - The system is running the Linux kernel.
  - All CPUs on the system are of the same type.
  - The CPU microarchitecture is Intel Nehalem or later. (Support for AMD
  processors is planned for the future.)
  - Each NUMA node on the system contains exactly one CPU.

Where possible, the library statically checks to ensure that these assumptions
hold. Otherwise, exceptions are thrown during runtime.

# Usage

This library is not ready for production yet! If you want to see some example
usage, look at [this file](test/system_query_test.cpp).

# Future Features

- Add support for AMD.

- Support the following auxiliary (PCIe) devices:
  - GPUs
  - MICs

- Support the following NUMA devices:
  - MICs
