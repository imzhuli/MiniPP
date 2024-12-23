#include "./relay_connection_manager.hpp"

bool xRelayConnectionManager::Init(size_t MaxConnectionSize) {
    Ticker.Update();
    return ContextPool.Init(MaxConnectionSize);
}

void xRelayConnectionManager::Clean() {
    ContextPool.Clean();
}

void xRelayConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
}

auto xRelayConnectionManager::Create() -> xRelayConnectionContext * {
    auto Id = ContextPool.Acquire();
    if (!Id) {
        return nullptr;
    }
    auto & Ref                = ContextPool[Id];
    Ref.RelaySideConnectionId = Id;
    return &Ref;
}

auto xRelayConnectionManager::GetConnectionById(uint64_t RelaySideConnectionId) -> xRelayConnectionContext * {
    return ContextPool.CheckAndGet(RelaySideConnectionId);
}

void xRelayConnectionManager::Destroy(xRelayConnectionContext * RCC) {
    ContextPool.CheckAndRelease(RCC->RelaySideConnectionId);
}