/*
** File Name: vendor.hpp
** Author:    Aditya Ramesh
** Date:      07/29/2014
** Contact:   _@adityaramesh.com
*/

#ifndef Z5E2DFDCA_1545_4FA9_ACF9_26A7AD12FA23
#define Z5E2DFDCA_1545_4FA9_ACF9_26A7AD12FA23

#include <ostream>
#include <ccbase/format.hpp>

namespace ctop {

enum class vendor
{
	intel,
	amd,
	unknown,
};

std::ostream& operator<<(std::ostream& os, const vendor& v)
{
	switch (v) {
	case vendor::intel:
		cc::write(os, "\"Intel\"");
		return os;
	case vendor::amd:
		cc::write(os, "\"AMD\"");
		return os;
	default:
		cc::write(os, "unknown");
		return os;
	}
}

}

#endif
