/*
** File Name:	cpuid_leaf.hpp
** Author:	Aditya Ramesh
** Date:	08/12/2014
** Contact:	_@adityaramesh.com
*/

#ifndef ZB7ECF656_BCC3_444F_B565_2869E3FF8B63
#define ZB7ECF656_BCC3_444F_B565_2869E3FF8B63

#include <ostream>
#include <stdexcept>

namespace ctop {

struct cpuid_leaf
{
	static constexpr auto basic_info                      = uint32_t{0x0};
	static constexpr auto version_info                    = uint32_t{0x1};
	static constexpr auto cache_tlb_info                  = uint32_t{0x2};
	static constexpr auto serial_number                   = uint32_t{0x3};
	static constexpr auto enumerable_cache_info           = uint32_t{0x4};
	static constexpr auto monitor_mwait_info              = uint32_t{0x5};
	static constexpr auto thermal_power_management_info   = uint32_t{0x6};
	static constexpr auto enumerable_feature_info         = uint32_t{0x7};
	static constexpr auto direct_cache_access_info        = uint32_t{0x9};
	static constexpr auto arch_perf_monitoring_info       = uint32_t{0xA};
	static constexpr auto enumerable_topology_info        = uint32_t{0xB};
	static constexpr auto enumerable_state_info           = uint32_t{0xD};
	static constexpr auto enumerable_qos_monitoring_info  = uint32_t{0xF};
	static constexpr auto enumerable_qos_enforcement_info = uint32_t{0x10};
	static constexpr auto enumerable_trace_info           = uint32_t{0x14};
	static constexpr auto max_extended_leaf               = uint32_t{0x80000000};
	static constexpr auto extended_feature_info           = uint32_t{0x80000001};
	static constexpr auto brand_string_part_1             = uint32_t{0x80000002};
	static constexpr auto brand_string_part_2             = uint32_t{0x80000003};
	static constexpr auto brand_string_part_3             = uint32_t{0x80000004};
	static constexpr auto tsc_info                        = uint32_t{0x80000006};
	static constexpr auto address_info                    = uint32_t{0x80000008};

	static std::string to_string(uint32_t leaf)
	{
		switch (leaf) {
		case basic_info:                      return "basic_info";
		case version_info:                    return "version_info";
		case cache_tlb_info:                  return "cache_tlb_info";
		case serial_number:                   return "serial_number";
		case enumerable_cache_info:           return "enumerable_cache_info";
		case monitor_mwait_info:              return "monitor_mwait_info";
		case thermal_power_management_info:   return "thermal_power_management_info";
		case enumerable_feature_info:         return "enumerable_feature_info";
		case direct_cache_access_info:        return "direct_cache_access_info";
		case arch_perf_monitoring_info:       return "arch_perf_monitoring_info";
		case enumerable_topology_info:        return "enumerable_topology_info";
		case enumerable_state_info:           return "enumerable_state_info";
		case enumerable_qos_monitoring_info:  return "enumerable_qos_monitoring_info";
		case enumerable_qos_enforcement_info: return "enumerable_qos_enforcement_info";
		case enumerable_trace_info:           return "enumerable_trace_info";
		case max_extended_leaf:               return "max_extended_leaf";
		case extended_feature_info:           return "extended_feature_info";
		case brand_string_part_1:             return "brand_string_part_1";
		case brand_string_part_2:             return "brand_string_part_2";
		case brand_string_part_3:             return "brand_string_part_3";
		case tsc_info:                        return "tsc_info";
		case address_info:                    return "address_info";
		default:
			throw std::logic_error{
				cc::format("Unknown leaf index ${hex, base}.", leaf)
			};
		}
	}
};

}

#endif
