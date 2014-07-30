<!--
  ** File Name: topology_enumeration.md
  ** Author:    Aditya Ramesh
  ** Date:      07/29/2014
  ** Contact:   _@adityaramesh.com
-->

# Introduction

These notes were taken on the [following whitepaper][topology_enumeration].

[topology_enumeration]:
https://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration
"Intel 64 Architecture Processor Topology Enumeration"

- Each logical processor in an Intel 64 or IA-32 platform supporting coherent
memory is assigned a unique APIC ID. A mulit-node installation may employ
vendor-specific BIOS that preserves the APIC IDs between hardware resets.
- The APIC ID is composed of three sub-fields: "SMT", "processor core" (or
"core"), and "physical package" (or "package").
- Each subfield corresponnds to a different hierarchial level.
- Each subfield at a given hierarchical level is extracted from the APIC ID
using the following parameters:
  - The width of the mask that can be used to mask off unneeded bits in the APIC
  ID.
  - An offset relative to bit 0 of the APIC ID.

- If the width for the SMT field is 0, then there is one logical processor
within the next outer layer of the hierarchy. If SMT is 1, then there are two
logical processors.
- If the width for the core field is 0, then there is only one processor core
within a physical processor. If the field is 1, then there are two.
- The APIC IDs that are assigned to each logical processor in the system need
not be contiguous. But the subsets of the bit fields corresponding to the three
hierarchical levels are contiguous at the bit boundary.
