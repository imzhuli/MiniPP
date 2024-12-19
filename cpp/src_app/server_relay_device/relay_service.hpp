#pragma once
#include <pp_common/_.hpp>
//
#include "./connection_manager.hpp"
#include "./device_manager.hpp"

class xDeviceRelayService
	: public xTcpServer::iListener
	, public xTcpConnection::iListener {
public:
	bool Init(xIoContext * CP, xNetAddress ControllerBindAddress, xNetAddress DataBindAddress);
	void Clean();
	void Tick(uint64_t NowMS);

	void OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
	void OnNewControlConnection(xSocket && NativeHandle);
	void OnNewDataConnection(xSocket && NativeHandle);
	void OnNewProxyConnection(xSocket && NativeHandle);

	void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

	bool OnPacket(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize);
	bool OnTerminalInitCtrlStream(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize);
	bool OnTerminalInitDataStream(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize);
	bool OnTerminalTargetConnectionUpdate(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize);

protected:
	void RemoveDeviceFromConnection(xConnection * Conn);
	void RemoveDevice(xDevice * Device);  // TODO
	bool PostConnectionData(xDevice * Device, uint32_t TerminalSideConnectionId, uint64_t LocalConnectionId, const ubyte * PayloadPtr, size_t PayloadSize);

	//
	xIoContext * IoContext = nullptr;
	xTicker      Ticker;
	xTcpServer   ControlServer;
	xTcpServer   DataServer;
	xTcpServer   ProxyServer;
};
