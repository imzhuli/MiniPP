#include "./relay_connection_manager.hpp"

bool xRelayConnectionManager::Init(xIoContext * ICP) {
    this->ICP = ICP;
    Ticker.Update();

    for (auto & CA : ConnectionArray) {
        CA = nullptr;
    }
    return true;
}

void xRelayConnectionManager::Clean() {
    KillConnectionList.GrabListTail(KeepAliveList);
    while (auto PC = static_cast<xPA_RelayConnection *>(KillConnectionList.PopHead())) {
        DestroyConnection(PC);
    }
}

void xRelayConnectionManager::Tick() {
    Ticker.Update();
    Todo("do keepalive, and close idle connections");

    while (auto PC = static_cast<xPA_RelayConnection *>(KillConnectionList.PopHead())) {
        DestroyConnection(PC);
    }
}

xPA_RelayConnection * xRelayConnectionManager::GetRelayConnectionById(uint32_t LocalConnectionId) {
    assert(LocalConnectionId < MAX_CONNECTIONS);
    return ConnectionArray[LocalConnectionId];
}

uint32_t xRelayConnectionManager::AcquireConnectionId() {
    uint32_t newId = ConnectionIdManager.Acquire();
    if (!newId) {
        return 0;
    }
    if (newId >= MAX_CONNECTIONS) {
        ConnectionIdManager.Release(newId);
        return 0;
    }
    return newId;
}

void xRelayConnectionManager::ReleaseConnectionId(uint32_t LocalConnectionId) {
    ConnectionIdManager.Release(LocalConnectionId);
}

xPA_RelayConnection * xRelayConnectionManager::AcquireRelayConnection(const xNetAddress & TargetAddress) {
    auto & Header = ConnectionSelectMap[TargetAddress];
    auto   RCP    = static_cast<xPA_RelayConnection *>(Header.PopHead());
    if (RCP) {
        Header.AddTail(*RCP);
        return RCP;
    }

    return nullptr;
}

void xRelayConnectionManager::DeferKillConnection(xPA_RelayConnection * PC) {
    PC->MarkDeleted = true;
    KillConnectionList.GrabTail(*PC);
}

xPA_RelayConnection * xRelayConnectionManager::CreateConnection(const xNetAddress & TargetAddress) {
    auto CID = ConnectionIdManager.Acquire();
    if (!CID) {
        X_DEBUG_PRINTF("out of connection id");
        return nullptr;
    }
    if (CID >= MAX_CONNECTIONS) {
        X_DEBUG_PRINTF("maximum connection number exceeded");
        ConnectionIdManager.Release(CID);
        return nullptr;
    }
    auto PC = new xPA_RelayConnection();
    if (!PC) {
        ConnectionIdManager.Release(CID);
        return nullptr;
    }

    if (!PC->Init(ICP, TargetAddress, this)) {
        ConnectionIdManager.Release(CID);
        delete PC;
        return nullptr;
    }

    PC->LocalConnectionId = CID;
    KeepAlive(PC);
    return PC;
}

void xRelayConnectionManager::DestroyConnection(xPA_RelayConnection * PC) {
    PC->Clean();
    delete PC;
}

void xRelayConnectionManager::KeepAlive(xPA_RelayConnection * PC) {
    if (!PC->MarkDeleted) {
        return;
    }
    PC->KeepAliveTimestampMS = Ticker();
    KeepAliveList.GrabTail(*PC);
}

void xRelayConnectionManager::OnConnected(xTcpConnection * TcpConnectionPtr) {
    TcpConnectionPtr->PostRequestKeepAlive();
}

void xRelayConnectionManager::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    DeferKillConnection(static_cast<xPA_RelayConnection *>(TcpConnectionPtr));
}

size_t xRelayConnectionManager::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    return DataSize;
}
