#pragma once
#include <pp_common/_.hpp>

class xConnection
    : public xTcpConnection
    , public xListNode {
public:
    static constexpr const uint64_t MARK_DELETE = 0x8F;

    uint64_t ConnectionId   = 0;
    uint64_t IdleTimestamMS = 0;
    uint64_t Marks          = 0;

    xVariable UserContext   = {};
    xVariable UserContextEx = {};

    bool PostPacket(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
};

class xRL_DeviceConnectionManager {

public:
    bool Init(xIoContext * ICP, size32_t MaxConnections = 50'0000);
    void Clean();

    void Tick(uint64_t NowMS);
    void RemoveIdleConnections();

    xConnection * AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void          DeferReleaseConnection(xConnection * Conn);
    void          KeepAlive(xConnection * Conn);

    xConnection * GetConnectionById(uint64_t ConnectionId) {
        auto PR = ConnectionIdManager.CheckAndGet(ConnectionId);
        if (!PR) {
            return nullptr;
        }
        return *PR;
    }

private:
    xConnection * CreateConnection();
    void          DestroyConnection(xConnection * Conn);
    void          FreeAllConnections();

private:
    xIoContext *                   ICP;
    xTicker                        Ticker;
    uint64_t                       IdleTimeoutMS = 120'000;
    xList<xConnection>             NewConnectionList;
    xList<xConnection>             IdleConnectionList;
    xList<xConnection>             KillConnectionList;
    xIndexedStorage<xConnection *> ConnectionIdManager;
};
