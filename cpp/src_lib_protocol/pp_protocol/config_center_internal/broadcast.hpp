#pragma once
#include <pp_common/base.hpp>
#include <pp_common/error.hpp>

struct xCC_RelayServerInfo {
    uint32_t    RelayServerId;
    xNetAddress DeviceCtrlServerAddress;
    xNetAddress DeviceDataServerAddress;
    xNetAddress ProxyServerAddress;
};

class xCC_BroadCastRelayServerInfo : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(ChallengeString);
    }
    void DeserializeMembers() override {
        R(ChallengeString);
    }

public:
    std::string ChallengeString;
};
