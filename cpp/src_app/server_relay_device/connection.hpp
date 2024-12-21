#pragma once
#include <pp_common/_.hpp>

class xRD_ConnectionBase : public xTcpConnection {
public:
    static constexpr const uint64_t FLAG_NONE         = 0;
    static constexpr const uint64_t FLAG_CTRL         = 0x01;
    static constexpr const uint64_t FLAG_DATA         = 0x02;
    static constexpr const uint64_t FLAG_PROXY_CLIENT = 0x04;

    static constexpr const uint64_t FLAG_TYPE_MASK = 0x00FF;

public:
    uint64_t  ConnectionId  = 0;
    uint64_t  Flags         = FLAG_NONE;
    xVariable UserContext   = {};
    xVariable UserContextEx = {};

    bool IsType_Ctrl() {
        return Flags & FLAG_CTRL;
    }
    void SetType_Ctrl() {
        Flags &= !FLAG_TYPE_MASK;
        Flags |= FLAG_CTRL;
    }
    bool IsType_Data() {
        return Flags & FLAG_DATA;
    }
    void SetType_Data() {
        Flags &= !FLAG_TYPE_MASK;
        Flags |= FLAG_DATA;
    }
    bool IsType_ProxyClient() {
        return Flags & FLAG_PROXY_CLIENT;
    }
    void SetType_ProxyClient() {
        Flags &= !FLAG_TYPE_MASK;
        Flags |= FLAG_PROXY_CLIENT;
    }

    bool PostPacket(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
};