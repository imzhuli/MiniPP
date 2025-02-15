#pragma once
#include <core/core_min.hpp>
#include <core/list.hpp>
#include <network/net_address.hpp>
#include <pp_common/region.hpp>
#include <unordered_map>
#include <vector>

struct xCC_DnsDispatcherInfo {
    xel::xNetAddress BindAddress;
};

//
struct xCC_DeviceRelayServerInfoBase {
    uint32_t         ServerId;
    xRegionId        RegionId;
    xel::xNetAddress DataAddress;
    xel::xNetAddress CtrlAddress;
    bool             IsStaticRelayServer;
};

struct xCC_DeviceCountryNode : xListNode {};

struct xCC_DeviceRelayServerInfo final : xCC_DeviceCountryNode {
    xCC_DeviceRelayServerInfoBase BaseInfo;
};
using xCC_DeviceRelayServerInfoList = xel::xList<xCC_DeviceRelayServerInfo>;

struct xCC_GeoIpServerInfo {
    uint32_t         CountryId;
    xel::xNetAddress BindAddress;
};

struct xCC_Root {
    std::vector<xCC_DnsDispatcherInfo>                          DnsDispatchers;
    std::unordered_map<uint32_t, xCC_DeviceRelayServerInfoList> DeviceRelayServers;  // CountryId -> DeviceRelayerServer
    std::unordered_map<uint32_t, xCC_GeoIpServerInfo>           GeoIpServers;        //
};
