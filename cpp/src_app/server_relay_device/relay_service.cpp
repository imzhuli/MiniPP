#include "./relay_service.hpp"

#include "./connection.hpp"
#include "./global.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/relay_terminal/connection.hpp>
#include <pp_protocol/relay_terminal/init_ctrl_stream.hpp>
#include <pp_protocol/relay_terminal/init_data_stream.hpp>
#include <pp_protocol/relay_terminal/post_data.hpp>

bool xDeviceRelayService::Init(xIoContext * CP, xNetAddress ControllerBindAddress, xNetAddress DataBindAddress, xNetAddress ProxyBindAddress) {
    RuntimeAssert(ControlServer.Init(CP, ControllerBindAddress, this));
    RuntimeAssert(DataServer.Init(CP, DataBindAddress, this));
    RuntimeAssert(ProxyServer.Init(CP, ProxyBindAddress, this));
    return true;
}

void xDeviceRelayService::Clean() {
    ControlServer.Clean();
    DataServer.Clean();
    ProxyServer.Clean();
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
    auto Conn = DeviceConnectionManager.AcceptConnection(std::move(NativeHandle), this);
    Conn->SetType_Ctrl();
}

void xDeviceRelayService::OnNewDataConnection(xSocket && NativeHandle) {
    auto Conn = DeviceConnectionManager.AcceptConnection(std::move(NativeHandle), this);
    Conn->SetType_Data();
}

void xDeviceRelayService::OnNewProxyConnection(xSocket && NativeHandle) {
    Todo("require proxy connection");
}

void xDeviceRelayService::OnConnected(xTcpConnection * TcpConnectionPtr) {
    Fatal("should not be called");
}

void xDeviceRelayService::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto Conn = static_cast<xRD_ConnectionBase *>(TcpConnectionPtr);
    if (Conn->IsType_Ctrl()) {
        auto DC = static_cast<xRD_DeviceConnection *>(Conn);
        RemoveDeviceFromConnection(DC);
        DeviceConnectionManager.DeferReleaseConnection(DC);
        return;
    } else if (Conn->IsType_Data()) {
        auto DC = static_cast<xRD_DeviceConnection *>(Conn);
        DeviceConnectionManager.DeferReleaseConnection(DC);
        return;
    }
    assert(Conn->IsType_ProxyClient());
    auto PC = static_cast<xRD_ProxyConnection *>(Conn);
    ProxyConnectionManager.DeferReleaseConnection(PC);
}

size_t xDeviceRelayService::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    auto   Conn       = static_cast<xRD_ConnectionBase *>(TcpConnectionPtr);
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
            if (Conn->IsType_Ctrl() || Conn->IsType_Data()) {
                DeviceConnectionManager.KeepAlive(static_cast<xRD_DeviceConnection *>(Conn));
            } else {
                assert(Conn->IsType_ProxyClient());
                ProxyConnectionManager.KeepAlive(static_cast<xRD_ProxyConnection *>(Conn));
            }
        } else {
            auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
            auto PayloadSize = Header.GetPayloadSize();
            // dispatch packet
            if (Conn->IsType_Ctrl()) {
                if (!OnCtrlPacket(static_cast<xRD_DeviceConnection *>(Conn), Header, PayloadPtr, PayloadSize)) { /* packet error */
                    return InvalidDataSize;
                }
            } else if (Conn->IsType_Data()) {
                if (!OnDataPacket(static_cast<xRD_DeviceConnection *>(Conn), Header, PayloadPtr, PayloadSize)) { /* packet error */
                    return InvalidDataSize;
                }
            } else {
                assert(Conn->IsType_ProxyClient());
                if (!OnProxyPacket(static_cast<xRD_ProxyConnection *>(Conn), Header, PayloadPtr, PayloadSize)) { /* packet error */
                    return InvalidDataSize;
                }
            }
        }
        DataPtr    += PacketSize;
        RemainSize -= PacketSize;
    }
    return DataSize - RemainSize;
}

bool xDeviceRelayService::OnCtrlPacket(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("Cmd=%" PRIx64 ", Request body: \n%s", Header.CommandId, HexShow(Payload, PayloadSize).c_str());
    switch (Header.CommandId) {
        case Cmd_Terminal_RL_InitCtrlStream: {
            return OnTerminalInitCtrlStream(Conn, Header, Payload, PayloadSize);
        }
    }
    return false;
}

bool xDeviceRelayService::OnDataPacket(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("Cmd=%" PRIx64 ", Request body: \n%s", Header.CommandId, HexShow(Payload, PayloadSize).c_str());
    switch (Header.CommandId) {
        case Cmd_Terminal_RL_InitDataStream: {
            return OnTerminalInitDataStream(Conn, Header, Payload, PayloadSize);
        }
        case Cmd_Terminal_RL_NotifyConnectionState: {
            return OnTerminalTargetConnectionUpdate(Conn, Header, Payload, PayloadSize);
        }
        default:
            break;
    }
    return false;
}

bool xDeviceRelayService::OnProxyPacket(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    return true;
}

void xDeviceRelayService::RemoveDeviceFromConnection(xRD_DeviceConnection * Conn) {
    auto DeviceId = Conn->DeviceId;
    auto Device   = DeviceManager.GetDeviceById(DeviceId);
    if (!Device) {
        return;
    }
    DeviceManager.ReleaseDevice(Device);
}

void xDeviceRelayService::RemoveDevice(xDevice * Device) {
    if (Device->ControlConnection) {
        DeviceConnectionManager.DeferReleaseConnection(Steal(Device->ControlConnection));
    }
    if (Device->DataConnection) {
        DeviceConnectionManager.DeferReleaseConnection(Steal(Device->DataConnection));
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

bool xDeviceRelayService::OnTerminalInitCtrlStream(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
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
    DeviceConnectionManager.KeepAlive(Conn);
    return true;
}

bool xDeviceRelayService::OnTerminalInitDataStream(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto S = xInitDataStream();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    auto CtrlConn = DeviceConnectionManager.GetConnectionById(S.CtrlId);
    if (!CtrlConn) {
        X_DEBUG_PRINTF("no ctrl id conn found");
        return false;
    }

    auto R     = xInitDataStreamResp();
    R.Accepted = true;

    // accept data stream and move it to long idle list
    Conn->PostPacket(Cmd_Terminal_RL_InitDataStreamResp, Header.RequestId, R);
    DeviceConnectionManager.KeepAlive(Conn);

    // do test:

    auto baidu               = xNetAddress::Parse("183.2.172.42:80");
    auto NC                  = xTR_CreateConnection();
    NC.RelaySideConnectionId = 1024;
    NC.TargetAddress         = baidu;
    CtrlConn->PostPacket(Cmd_Terminal_RL_CreateConnection, 0, NC);

    return true;
}

bool xDeviceRelayService::OnTerminalTargetConnectionUpdate(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
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
