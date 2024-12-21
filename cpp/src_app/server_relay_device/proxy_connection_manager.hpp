#pragma once
#include "./connection.hpp"

#include <pp_common/_.hpp>

struct xRD_ProxyIdleNode : xListNode {};

struct xRD_ProxyConnection
    : xRD_ConnectionBase
    , xRD_ProxyIdleNode {

    //
};

struct xRD_ProxyConnectionManager {

    xRD_ProxyConnection * AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void                  DeferReleaseConnection(xRD_ProxyConnection * Conn);
    void                  KeepAlive(xRD_ProxyConnection * Conn);
    //
};
