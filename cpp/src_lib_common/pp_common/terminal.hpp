#pragma once
#include "./base.hpp"
#include "./region.hpp"

#include <unordered_map>

struct xTerminalInfo {
	xTerminalId xerminalId = 0;  // internal usage

	xRegionId   RegionId;
	xNetAddress TerminalAddress;             // terminal out address
	xNetAddress TerminalControllerAddress;   // relay server, or terminal service address
	uint64_t    TerminalControllerSubIndex;  // index in relay server
};

struct xTerminalControllerBinding {
	xNetAddress TargetAddress;
	xNetAddress TerminalControllerAddress;
	uint64_t    TerminalId;
};
