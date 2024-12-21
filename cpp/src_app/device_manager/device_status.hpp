#pragma once
#include <pp_common/_.hpp>

struct xDM_DeviceBaseNode : xListNode {};
struct xDM_CountryNode : xListNode {};
struct xDM_StateNode : xListNode {};
struct xDM_CityNode : xListNode {};

struct xDM_DeviceState
    : xDM_DeviceBaseNode
    , xDM_CountryNode
    , xDM_StateNode
    , xDM_CityNode {

    xNetAddress DeviceAddress;
    xNetAddress RelayServerProxyAddress;
    uint64_t    RelaySideDeviceId;
    uint32_t    HealthPoint = 0;  // 0-100

    //
};
