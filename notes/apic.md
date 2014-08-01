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

- On more recent processors, the x2APIC ID is also supported. This is primarily
used by CPUID leaf 11.
- The x2APIC ID is partitioned into several components, each of which
corresponds to a level in the processor topology.
- The topology might be structured as follows: SMT, core, processor package, and
cluster. The last component, if it exists, would be configured by the
vendor-specific BIOS.
- The number of bits to shift the x2APIC ID right to extract the ID
corresponding to each level of the hierarchy is given by CPUID leaf 11.

- Before determining the APIC ID assigned to each processor, we must determine
the number of logical processors that are currently accessible on the machine.
- Since the OS determines which logical processors are accessible to a given
process (some processors might be offline, say because of power management), the
means to do this are platform-dependent.

- Afterwards, we need to run the CPUID instruction on each logical processor in
the processor package.
- If x2APIC IDs are desired, then we need to run CPUID leaf 11.
