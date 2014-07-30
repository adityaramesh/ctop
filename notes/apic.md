<!--
  ** File Name: apic.md
  ** Author:    Aditya Ramesh
  ** Date:      07/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Introduction

These notes were taken on the [following article][htt_linux].

[htt_linux]:
https://software.intel.com/en-us/articles/methods-to-utilize-intels-hyper-threading-technology-with-linux
"Methods to Utilize Intel's Hyperthreading Technology with Linux"

# APIC IDs

- Each logical processor on the system is assigned a unique APIC ID after reset.
- The default APIC ID can be changed by the BIOS or OS. This can be useful in
cluster environments, where it is beneficial to associate the same logical
processor with the same APIC ID between hardware resets.

- Before determining the APIC ID assigned to each processor, we must determine
the number of logical processor in each processor package.
- To do this, we can execute leaf 1 of the CPUID instruction, check whether HTT
is supported, and examine a particular range of bytes in one of the output
registers.

- Now, we need to run the CPUID instruction on each logical processor in the
processor package.
- The APIC IDs are used by leaf 11 of the CPUID instruction to perform topology
enumeration.
