#pragma once
#include "./connection.hpp"

#include <pp_common/_.hpp>

struct xRD_ProxyConnection
    : xRD_ConnectionBase
    , xListNode {

    //
};

class xRD_ProxyConnectionManager {
public:
    bool Init(xIoContext * ICP, size32_t MaxRD_DeviceConnections);
    void Clean();

    void Tick(uint64_t NowMS);
    void RemoveIdleConnections();

    xRD_ProxyConnection * AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void                  DeferReleaseConnection(xRD_ProxyConnection * Conn);
    void                  KeepAlive(xRD_ProxyConnection * Conn);
    //
protected:
    xIoContext *                           ICP;
    xTicker                                Ticker;
    xList<xRD_ProxyConnection>             IdleConnectionList;
    xList<xRD_ProxyConnection>             KillConnectionList;
    xIndexedStorage<xRD_ProxyConnection *> ConnectionIdManager;
};
