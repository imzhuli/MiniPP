#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/init_ctrl_stream.hpp>
#include <pp_protocol/device_relay/init_data_stream.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

bool xDeviceRelayService::OnDataPacket(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("Cmd=%" PRIx64 ", Request body: \n%s", Header.CommandId, HexShow(Payload, PayloadSize).c_str());
    switch (Header.CommandId) {
        case Cmd_Terminal_RL_InitDataStream: {
            return OnTerminalInitDataStream(Conn, Header, Payload, PayloadSize);
        }
        case Cmd_Terminal_RL_NotifyConnectionState: {
            return OnTerminalTargetConnectionUpdate(Conn, Header, Payload, PayloadSize);
        }
        case Cmd_Terminal_RL_PostData: {
            return OnTerminalPostData(Conn, Header, Payload, PayloadSize);
        }
        default:
            break;
    }
    return false;
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
    if (CtrlConn->DeviceId) {
        X_DEBUG_PRINTF("duplicate device connection");
        return false;
    }

    auto NewDevice = DeviceManager.NewDevice();
    if (!NewDevice) {
        X_DEBUG_PRINTF("failed to create device context");
        DeviceConnectionManager.DeferReleaseConnection(CtrlConn);
        return false;
    }

    auto R     = xInitDataStreamResp();
    R.Accepted = true;

    // accept data stream and move it to long idle list
    Conn->PostPacket(Cmd_Terminal_RL_InitDataStreamResp, Header.RequestId, R);

    X_DEBUG_PRINTF("device accepted, DeviceRuntimeId:%" PRIu64 "", NewDevice->DeviceRuntimeId);
    Conn->DeviceId            = NewDevice->DeviceRuntimeId;
    CtrlConn->DeviceId        = NewDevice->DeviceRuntimeId;
    NewDevice->CtrlConnection = CtrlConn;
    NewDevice->DataConnection = Conn;
    DeviceConnectionManager.KeepAlive(Conn);
    return true;
}

bool xDeviceRelayService::OnTerminalTargetConnectionUpdate(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto S = xTR_ConnectionStateNotify();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }

    X_DEBUG_PRINTF(
        "New ConnectionState: %s terminalSideCid=%" PRIx32 ", relaySideCid=%" PRIx64 ", tR=%" PRIu64 ", tW=%" PRIu64 "",
        xTR_ConnectionStateNotify::GetStateName(S.NewState), S.DeviceSideConnectionId, S.RelaySideConnectionId, S.TotalReadBytes, S.TotalWrittenBytes
    );

    auto CR = RelayConnectionManager.GetConnectionById(S.RelaySideConnectionId);
    if (!CR || CR->RelaySideConnectionId != S.RelaySideConnectionId) {
        X_DEBUG_PRINTF("Connection not found: Id=%" PRIx64 "", S.RelaySideConnectionId);
        return true;
    }

    auto F = xPR_ConnectionStateNotify();
    switch (S.NewState) {
        case xTR_ConnectionStateNotify::STATE_ESTABLISHED:
            CR->DeviceSideConnectionId = S.DeviceSideConnectionId;

            F.NewState = xPR_ConnectionStateNotify::STATE_ESTABLISHED;
            break;
        case xTR_ConnectionStateNotify::STATE_UPDATE_TRANSFER:
            F.NewState = xPR_ConnectionStateNotify::STATE_UPDATE_TRANSFER;
            break;
        case xTR_ConnectionStateNotify::STATE_CLOSED:
            F.NewState = xPR_ConnectionStateNotify::STATE_CLOSED;
            break;
        default:
            X_DEBUG_PRINTF("Unrecognized state");
            return false;
    }
    F.RelaySideConnectionId = CR->RelaySideConnectionId;
    F.ProxySideConnectionId = CR->ProxySideConnectionId;

    auto PAConn = ProxyConnectionManager.GetConnectionById(CR->ProxyId);
    if (!PAConn) {
        X_DEBUG_PRINTF("proxy not found");
        // TODO 删除此连接
        return true;
    }
    PAConn->PostPacket(Cmd_PA_RL_NotifyConnectionState, 0, F);
    return true;
}

bool xDeviceRelayService::OnTerminalPostData(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto S = xTR_PostData();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }

    X_DEBUG_PRINTF("terminalSideCid=%" PRIx32 ", relaySideCid=%" PRIx64 ", size=%zi", S.DeviceSideConnectionId, S.RelaySideConnectionId, S.PayloadView.size());
    auto CR = RelayConnectionManager.GetConnectionById(S.RelaySideConnectionId);
    if (!CR || CR->RelaySideConnectionId != S.RelaySideConnectionId) {
        X_DEBUG_PRINTF("Connection not found: Id=%" PRIx64 "", S.RelaySideConnectionId);
        return true;
    }

    auto PAConn = ProxyConnectionManager.GetConnectionById(CR->ProxyId);
    if (!PAConn) {
        X_DEBUG_PRINTF("proxy not found");
        return true;
    }

    auto Push                  = xPR_PushData();
    Push.RelaySideConnectionId = CR->RelaySideConnectionId;
    Push.ProxySideConnectionId = CR->ProxySideConnectionId;
    Push.PayloadView           = S.PayloadView;
    PAConn->PostPacket(Cmd_PA_RL_PostData, 0, Push);

    return true;
}
