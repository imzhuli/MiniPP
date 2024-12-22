#pragma once
#include <pp_common/_.hpp>

struct xRelayConnectionContext : xListNode {
    uint64_t RelaySideConnectionId;
    uint64_t DeviceSideConnectionId;
    uint64_t ProxySideConnectionId;
};

class xRelayConnectionManager {
public:
    bool Init(size_t MaxConnectionSize);
    void Clean();

private:
    xIndexedStorage<xRelayConnectionContext> ContextPool;
};
