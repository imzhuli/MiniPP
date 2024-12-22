#include "./relay_connection_manager.hpp"

bool xRelayConnectionManager::Init(size_t MaxConnectionSize) {
    return ContextPool.Init(MaxConnectionSize);
}

void xRelayConnectionManager::Clean() {
    ContextPool.Clean();
}
