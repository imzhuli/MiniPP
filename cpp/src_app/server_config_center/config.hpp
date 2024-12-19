#pragma once
#include "./typedef.hpp"

#include <pp_common/_.hpp>
#include <vector>

extern std::vector<xCC_DeviceRelayServerInfoBase> ForceRelayServerList;

extern bool LoadConfig(const char * filename);

/********************************
 *
 *   TEST
 *
 ********************************/
extern xCC_DeviceRelayServerInfoBase RelayForTest;
