#pragma once
#include <pp_common/_.hpp>

struct xPA_LastUsedNode : xListNode {};
struct xPA_KeepAliveNode : xListNode {};

struct xPA_RelayConnection
    : xTcpConnection
    , xPA_LastUsedNode
    , xPA_KeepAliveNode {

    uint32_t    LocalConnectionId;
    uint64_t    KeepAliveTimestampMS;
    xNetAddress RelayServerAddress;

    //
    bool MarkDeleted;
};
