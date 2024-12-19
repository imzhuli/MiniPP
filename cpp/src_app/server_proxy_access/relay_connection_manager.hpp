#pragma once
#include "./relay_connection.hpp"

#include <map>
#include <pp_common/base.hpp>

class xRelayConnectionManager : xTcpConnection::iListener {
public:
    static constexpr const size_t MAX_CONNECTIONS = 4000;
    static_assert(MAX_CONNECTIONS < xObjectIdManagerMini::MaxObjectId);

    bool Init(xIoContext * ICP);
    void Clean();
    void Tick();

    xPA_RelayConnection * GetRelayConnectionById(uint32_t LocalConnectionId);
    xPA_RelayConnection * AcquireRelayConnection(const xNetAddress & TargetAddress);

protected:  // callbacks:
    void OnConnected(xTcpConnection * TcpConnectionPtr) override {
    }
    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override {
    }
    void OnFlush(xTcpConnection * TcpConnectionPtr) override {
    }
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override {
        return DataSize;
    }

protected:
    uint32_t              AcquireConnectionId();
    void                  ReleaseConnectionId(uint32_t LocalConnectionId);
    xPA_RelayConnection * CreateConnection(const xNetAddress & TargetAddress);
    void                  DestroyConnection(xPA_RelayConnection * PC);

    xIoContext *                                   ICP;
    xTicker                                        Ticker;
    xPA_RelayConnection *                          ConnectionArray[MAX_CONNECTIONS];
    xObjectIdManagerMini                           ConnectionIdManager;
    std::map<xNetAddress, xList<xPA_LastUsedNode>> ConnectionSelectMap;
};
