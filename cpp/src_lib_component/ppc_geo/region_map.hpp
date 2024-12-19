#pragma once

#include <pp_common/_.hpp>
#include <pp_common/region.hpp>

struct xPP_RegionInfo {
	xRegionId   RegionId;
	std::string CountryName;
	std::string StateName;
	std::string CityName;
};

void                   BuildRegionInfoMapFromCSV(const char * filename);
const xPP_RegionInfo * GetRegionInfoByFullName(const std::string & FullName);
const xPP_RegionInfo * GetRegionInfoByCityNameOnly(const std::string & CityName);
