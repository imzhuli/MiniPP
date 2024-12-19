#pragma once
#include <pp_common/_.hpp>
//
#include <object/object.hpp>
#include <vector>

struct xPP_ConnectionNode : xListNode {};
struct xPP_ConnectionKeepAliveNode : xListNode {
	uint64_t LastKeepAliveTimestampMS = 0;
};

class xPP_Connection
	: public xTcpConnection
	, public xPP_ConnectionNode
	, public xPP_ConnectionKeepAliveNode {
public:
	using xTcpConnection::Clean;
	using xTcpConnection::Init;

public:
	uint32_t    DebugConnectionId = 0;
	xNetAddress ServerAddress;
};

class xPP_Client : xTcpConnection::iListener {
public:
	bool Init(xIoContext * ICP);
	void Clean();
	void UpdateServerAddresses(const std::vector<xNetAddress> & ServerAddresses);

	// check connection
	void Tick();

protected:
	void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

	xPP_Connection * PickConnection();
	//
	virtual bool OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

	//
	xPP_Connection * AllocConnection();
	void             FreeConnection(xPP_Connection * PC);

protected:
	xIoContext *                       ICP = nullptr;
	xList<xPP_ConnectionNode>          ReadyClientNodeList;
	xList<xPP_ConnectionNode>          ConnectingNodeList;
	xList<xPP_ConnectionNode>          ReconnectClientNodeList;
	xList<xPP_ConnectionNode>          ClosingClientNodeList;
	xList<xPP_ConnectionKeepAliveNode> ReadyClientKeepAliveNodeList;
	xList<xPP_ConnectionKeepAliveNode> ReadyClientKeepAliveCheckingNodeList;
	std::vector<xNetAddress>           ServerAddresses;

	uint64_t NowMS                    = GetTimestampMS();
	uint64_t LastReconnectTimestampMS = 0;

	// debug:
	xel::xObjectIdManagerMini DebugIdManager;
};
