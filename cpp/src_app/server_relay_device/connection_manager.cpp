#include "./connection_manager.hpp"

/***************
 * Connection
 */
bool xConnection::PostPacket(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    ubyte Buffer[MaxPacketSize];
    auto  PSize = WritePacket(CmdId, RequestId, Buffer, Message);
    PostData(Buffer, PSize);
    return true;
}

/***************
 * Connection manager
 */

bool xRL_DeviceConnectionManager::Init(xIoContext * ICP, size32_t MaxConnections) {
    RuntimeAssert(this->ICP = ICP);
    RuntimeAssert(ConnectionIdManager.Init(MaxConnections));
    return true;
}
void xRL_DeviceConnectionManager::Clean() {
    FreeAllConnections();
    ConnectionIdManager.Clean();
    this->ICP = nullptr;
}

void xRL_DeviceConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    RemoveIdleConnections();
}

void xRL_DeviceConnectionManager::RemoveIdleConnections() {
    auto NewConnectionKillTimepoint = Ticker() - 3'000;
    while (auto TOC = NewConnectionList.PopHead([NewConnectionKillTimepoint](const xConnection & C) -> bool {
        return C.IdleTimestamMS <= NewConnectionKillTimepoint;
    })) {
        X_DEBUG_PRINTF("New Connection: %" PRIx64 "", TOC->ConnectionId);
        KillConnectionList.GrabTail(*TOC);
    }

    auto KillTimepoint = Ticker() - IdleTimeoutMS;
    while (auto TOC = IdleConnectionList.PopHead([KillTimepoint](const xConnection & C) -> bool { return C.IdleTimestamMS <= KillTimepoint; })) {
        KillConnectionList.GrabTail(*TOC);
    }

    while (auto TOC = KillConnectionList.PopHead()) {
        DestroyConnection(TOC);
    }
}

xConnection * xRL_DeviceConnectionManager::AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener) {
    auto C = CreateConnection();
    if (!C->Init(ICP, std::move(NativeHandle), Listener)) {
        X_DEBUG_PRINTF("Failed to accept connection");
        DestroyConnection(C);
    }
    X_DEBUG_PRINTF(" ConnectionId=%" PRIx64 "", C->ConnectionId);
    return C;
}

xConnection * xRL_DeviceConnectionManager::CreateConnection() {
    auto C = new (std::nothrow) xConnection();
    if (!C) {
        return nullptr;
    }
    auto Id = ConnectionIdManager.Acquire(C);
    if (!Id) {
        delete C;
        return nullptr;
    }
    C->ConnectionId   = Id;
    C->IdleTimestamMS = Ticker;
    NewConnectionList.AddTail(*C);
    return C;
}

void xRL_DeviceConnectionManager::DeferReleaseConnection(xConnection * Conn) {
    Conn->Marks |= xConnection::MARK_DELETE;
    KillConnectionList.GrabTail(*Conn);
}

void xRL_DeviceConnectionManager::KeepAlive(xConnection * Conn) {
    if (Conn->Marks & xConnection::MARK_DELETE) {
        return;
    }
    Conn->IdleTimestamMS = Ticker;
    IdleConnectionList.GrabTail(*Conn);
}

void xRL_DeviceConnectionManager::DestroyConnection(xConnection * Conn) {
    assert(Conn->ConnectionId);
    X_DEBUG_PRINTF("DestroyConnection: %" PRIx64 "", Conn->ConnectionId);
    RuntimeAssert(ConnectionIdManager.CheckAndRelease(Conn->ConnectionId));
    Conn->Clean();
    delete Conn;
}

void xRL_DeviceConnectionManager::FreeAllConnections() {
    KillConnectionList.GrabListTail(NewConnectionList);
    KillConnectionList.GrabListTail(IdleConnectionList);
    while (auto PC = KillConnectionList.PopHead()) {
        DestroyConnection(PC);
    }
}
