#pragma once
#include "./base.hpp"

X_BEGIN

struct xProxyDnsResult;
struct xProxyOption;
struct iProxyBase;
struct iProxyListner;

struct xProxyDnsResult {
	xNetAddress A4;
	xNetAddress A6;
};

struct xProxyOption final {
	std::string     Host;
	std::string     User;
	std::string     Pass;
	iProxyListner * Listener;
};

struct iProxyBase {
	virtual bool Init(xIoContext * ICP, const xProxyOption & Option) = 0;
	virtual void Clean()                                             = 0;
};

struct iProxyListner {
	virtual void OnConnected(iProxyBase * PBP) = 0;
};

X_END
