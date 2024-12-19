#pragma once

#include <pp_common/_.hpp>
#include <pp_common/region.hpp>

//
#include <map>

struct xPP_StaticIPGeo {
	xRegionId RegionId;
	struct {
		uint32_t Forced : 1;
	};
};

class xStaticIPManager {
public:
	bool                    Init();
	void                    Clean();
	void                    Reload(const char * filename);
	const xPP_StaticIPGeo * Query(const xNetAddress & IP);

protected:
	std::map<xNetAddress, xPP_StaticIPGeo> IPMap;

	//
};
