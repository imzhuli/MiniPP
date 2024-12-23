#pragma once
#include <pp_common/_.hpp>

struct xRelayConnectionContext : xListNode {
    uint64_t RelaySideConnectionId;
    uint64_t DeviceSideConnectionId;
    uint64_t ProxySideConnectionId;

    uint64_t DeviceId;
    uint64_t ProxyId;
};

class xRelayConnectionManager {
public:
    bool Init(size_t MaxConnectionSize);
    void Clean();
    void Tick(uint64_t NowMS);

    auto Create() -> xRelayConnectionContext *;
    auto GetConnectionById(uint64_t RelaySideConnectionId) -> xRelayConnectionContext *;
    void Destroy(xRelayConnectionContext * RCC);

private:
    xTicker                                  Ticker;
    xIndexedStorage<xRelayConnectionContext> ContextPool;
};
