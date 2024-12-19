#include "./relay_service.hpp"

#include "./global.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/relay_terminal/connection.hpp>
#include <pp_protocol/relay_terminal/init_ctrl_stream.hpp>
#include <pp_protocol/relay_terminal/init_data_stream.hpp>
#include <pp_protocol/relay_terminal/post_data.hpp>

namespace {

	struct xConnCtx {

		[[maybe_unused]] static constexpr const uint32_t FLAG_UNSPEC       = 0x00;
		[[maybe_unused]] static constexpr const uint32_t FLAG_CTRL         = 0x01;
		[[maybe_unused]] static constexpr const uint32_t FLAG_DATA         = 0x02;
		[[maybe_unused]] static constexpr const uint32_t FLAG_PROXY_CLIENT = 0x03;

		uint32_t Flags;
		uint32_t Reserved;
		uint64_t DeviceId = 0;

		static xConnCtx From(xConnection * Conn) {
			return xConnCtx{
				.Flags    = Conn->UserContext.UX,
				.Reserved = Conn->UserContext.UY,
				.DeviceId = Conn->UserContextEx.U64,
			};
		}

		static void UpdateFlags(xConnection * Conn, uint32_t Flags) {
			Conn->UserContext.UX = Flags;
		}

		static void UpdateDeviceId(xConnection * Conn, uint64_t DeviceId) {
			Conn->UserContextEx.U64 = DeviceId;
		}

		static uint32_t GetFlags(xConnection * Conn) {
			return Conn->UserContext.UX;
		}

		static uint64_t GetDeviceId(xConnection * Conn) {
			return Conn->UserContextEx.U64;
		}

		void Dump(xConnection * Conn) {
			Conn->UserContext.UX    = Flags;
			Conn->UserContext.UY    = Reserved;
			Conn->UserContextEx.U64 = DeviceId;
		}

		//
	};

}  // namespace

bool xDeviceRelayService::Init(xIoContext * CP, xNetAddress ControllerBindAddress, xNetAddress DataBindAddress) {
	RuntimeAssert(ControlServer.Init(CP, ControllerBindAddress, this));
	RuntimeAssert(DataServer.Init(CP, DataBindAddress, this));
	return true;
}

void xDeviceRelayService::Clean() {
	ControlServer.Clean();
	DataServer.Clean();
}

void xDeviceRelayService::Tick(uint64_t NowMS) {
	Ticker.Update(NowMS);
}

void xDeviceRelayService::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
	if (TcpServerPtr == &ControlServer) {
		return OnNewControlConnection(std::move(NativeHandle));
	} else if (TcpServerPtr == &DataServer) {
		return OnNewDataConnection(std::move(NativeHandle));
	} else if (TcpServerPtr == &ProxyServer) {
		return OnNewProxyConnection(std::move(NativeHandle));
	}
	Unreachable();
}

void xDeviceRelayService::OnNewControlConnection(xSocket && NativeHandle) {
	auto Conn = ConnectionManager.AcceptConnection(std::move(NativeHandle), this);
	xConnCtx::UpdateFlags(Conn, xConnCtx::FLAG_CTRL);
}

void xDeviceRelayService::OnNewDataConnection(xSocket && NativeHandle) {
	auto Conn = ConnectionManager.AcceptConnection(std::move(NativeHandle), this);
	xConnCtx::UpdateFlags(Conn, xConnCtx::FLAG_DATA);
}

void xDeviceRelayService::OnNewProxyConnection(xSocket && NativeHandle) {
	auto Conn = ConnectionManager.AcceptConnection(std::move(NativeHandle), this);
	xConnCtx::UpdateFlags(Conn, xConnCtx::FLAG_PROXY_CLIENT);
}

void xDeviceRelayService::OnConnected(xTcpConnection * TcpConnectionPtr) {
	Fatal("should not be called");
}

void xDeviceRelayService::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	auto Conn  = static_cast<xConnection *>(TcpConnectionPtr);
	auto Flags = xConnCtx::GetFlags(Conn);
	if (Flags & xConnCtx::FLAG_CTRL) {
		RemoveDeviceFromConnection(Conn);
		return;
	}
	ConnectionManager.DeferReleaseConnection(Conn);
}

size_t xDeviceRelayService::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
	auto   Conn       = static_cast<xConnection *>(TcpConnectionPtr);
	size_t RemainSize = DataSize;
	while (RemainSize >= PacketHeaderSize) {
		auto Header = xPacketHeader::Parse(DataPtr);
		if (!Header) { /* header error */
			return InvalidDataSize;
		}
		auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
		if (RemainSize < PacketSize) {        // wait for data
			break;
		}
		if (Header.IsRequestKeepAlive()) {
			Conn->PostKeepAlive();
			ConnectionManager.KeepAlive(Conn);
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnPacket(Conn, Header, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

bool xDeviceRelayService::OnPacket(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
	X_DEBUG_PRINTF("Cmd=%" PRIx64 ", Request body: \n%s", Header.CommandId, HexShow(Payload, PayloadSize).c_str());
	switch (Header.CommandId) {
		case Cmd_Terminal_RL_InitCtrlStream: {
			return OnTerminalInitCtrlStream(Conn, Header, Payload, PayloadSize);
		}
		case Cmd_Terminal_RL_InitDataStream: {
			return OnTerminalInitDataStream(Conn, Header, Payload, PayloadSize);
		}
		case Cmd_Terminal_RL_NotifyConnectionState: {
			return OnTerminalTargetConnectionUpdate(Conn, Header, Payload, PayloadSize);
		}
		default:
			break;
	}
	return true;
}

void xDeviceRelayService::RemoveDeviceFromConnection(xConnection * Conn) {
	auto DeviceId = xConnCtx::GetDeviceId(Conn);
	auto Device   = DeviceManager.GetDeviceById(DeviceId);
	if (!Device) {
		return;
	}
	DeviceManager.ReleaseDevice(Device);
}

void xDeviceRelayService::RemoveDevice(xDevice * Device) {
	if (Device->ControlConnection) {
		ConnectionManager.DeferReleaseConnection(Steal(Device->ControlConnection));
	}
	if (Device->DataConnection) {
		ConnectionManager.DeferReleaseConnection(Steal(Device->DataConnection));
	}
	DeviceManager.ReleaseDevice(Device);
}

bool xDeviceRelayService::PostConnectionData(
	xDevice * Device, uint32_t TerminalSideConnectionId, uint64_t LocalConnectionId, const ubyte * PayloadPtr, size_t TotalPayloadSize
) {
	while (TotalPayloadSize) {
		auto PayloadSize            = std::min((size32_t)TotalPayloadSize, xTR_PostData::MAX_PAYLOAD_SIZE);
		auto PP                     = xTR_PostData();
		PP.TerminalSideConnectionId = TerminalSideConnectionId;
		PP.RelaySideConnectionId    = LocalConnectionId;
		PP.PayloadView              = { (const char *)PayloadPtr, PayloadSize };

		PayloadPtr       += PayloadSize;
		TotalPayloadSize -= PayloadSize;
	}

	return false;
}

bool xDeviceRelayService::OnTerminalInitCtrlStream(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
	auto CF = xConnCtx::GetFlags(Conn);
	if (!(CF & xConnCtx::FLAG_CTRL)) {
		return false;
	}

	auto S = xInitCtrlStream();
	if (!S.Deserialize(Payload, PayloadSize)) {
		return false;
	}
	// TODO: check

	auto R      = xInitCtrlStreamResp();
	R.DeviceId  = 1024;
	R.CtrlId    = Conn->ConnectionId;
	R.DeviceKey = "hello world!";
	Conn->PostPacket(Cmd_Terminal_RL_InitCtrlStreamResp, Header.RequestId, R);
	ConnectionManager.KeepAlive(Conn);
	return true;
}

bool xDeviceRelayService::OnTerminalInitDataStream(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
	auto CF = xConnCtx::GetFlags(Conn);
	if (!(CF & xConnCtx::FLAG_DATA)) {
		return false;
	}

	auto S = xInitDataStream();
	if (!S.Deserialize(Payload, PayloadSize)) {
		return false;
	}
	auto CtrlConn = ConnectionManager.GetConnectionById(S.CtrlId);
	if (!CtrlConn) {
		X_DEBUG_PRINTF("no ctrl id conn found");
		return false;
	}

	auto R     = xInitDataStreamResp();
	R.Accepted = true;

	// accept data stream and move it to long idle list
	Conn->PostPacket(Cmd_Terminal_RL_InitDataStreamResp, Header.RequestId, R);
	ConnectionManager.KeepAlive(Conn);

	// do test:

	auto baidu               = xNetAddress::Parse("183.2.172.42:80");
	auto NC                  = xTR_CreateConnection();
	NC.RelaySideConnectionId = 1024;
	NC.TargetAddress         = baidu;
	CtrlConn->PostPacket(Cmd_Terminal_RL_CreateConnection, 0, NC);

	return true;
}

bool xDeviceRelayService::OnTerminalTargetConnectionUpdate(xConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
	auto CF = xConnCtx::GetFlags(Conn);
	if (!(CF & xConnCtx::FLAG_DATA)) {
		return false;
	}

	auto S = xTR_ConnectionStateNotify();
	if (!S.Deserialize(Payload, PayloadSize)) {
		return false;
	}

	X_DEBUG_PRINTF(
		"New ConnectionState: %s terminalSideCid=%" PRIx32 ", relaySideCid=%" PRIx64 ", tR=%" PRIu64 ", tW=%" PRIu64 "",
		xTR_ConnectionStateNotify::GetStateName(S.NewState), S.TerminalSideConnectionId, S.RelaySideConnectionId, S.TotalReadBytes, S.TotalWrittenBytes
	);

	return true;
}
