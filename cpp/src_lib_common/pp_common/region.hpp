#pragma once
#include "./base.hpp"

struct xRegionId {
	xCountryId CountryId;
	xStateId   StateId;
	xCityId    CityId;
};

struct xRegionInfo {
	xRegionId RegionId;

	string CountryName;
	string CityName;
	string ShortCityName;
};

extern uint32_t CountryCodeToCountryId(const char * CC);
