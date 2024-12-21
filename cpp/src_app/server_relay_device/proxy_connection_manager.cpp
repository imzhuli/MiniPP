#include "./proxy_connection_manager.hpp"

bool xRD_ProxyConnectionManager::Init(xIoContext * ICP, size32_t MaxConnections) {
    RuntimeAssert(this->ICP = ICP);
    RuntimeAssert(ConnectionIdManager.Init(MaxConnections));
    Ticker.Update();
    return true;
}

void xRD_ProxyConnectionManager::Clean() {
    Todo("delete all connections");
    ConnectionIdManager.Clean();
    this->ICP = nullptr;
}

void xRD_ProxyConnectionManager::KeepAlive(xRD_ProxyConnection * Conn) {
    if (Conn->HasMark_Delete()) {
        return;
    }
    Conn->IdleTimestamMS = Ticker;
    IdleConnectionList.GrabTail(*Conn);
}

void xRD_ProxyConnectionManager::DeferReleaseConnection(xRD_ProxyConnection * Conn) {
    if (Conn->HasMark_Delete()) {
        return;
    }
    Conn->Mark_Delete();
    KillConnectionList.GrabTail(*Conn);
}