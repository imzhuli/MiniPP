#pragma once
#include <pp_common/_.hpp>
//
#include <object/object.hpp>

//
#include <array>
#include <string>
//

struct xRelayDeviceServerInfo {
    std::string ServerUuid;
    xNetAddress CtrlAddress;
    xNetAddress DataAddress;
};

class xRelayDeviceServerInfoList {
public:
    using xServerIndexKey = uint64_t;

private:
    struct xRelayDeviceServerInfoInternal {
        xServerIndexKey        IndexKey;
        xRelayDeviceServerInfo BaseInfo;
    };

    xServerIndexKey AddServerInfo(const xRelayDeviceServerInfo &);

private:
    uint32_t                                         Version = 0;
    std::array<xRelayDeviceServerInfoInternal, 1024> ServerInfoList;
};
