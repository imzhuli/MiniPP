#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/relay_terminal/connection.hpp>
#include <pp_protocol/relay_terminal/init_ctrl_stream.hpp>
#include <pp_protocol/relay_terminal/init_data_stream.hpp>
#include <pp_protocol/relay_terminal/post_data.hpp>

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
        xTR_ConnectionStateNotify::GetStateName(S.NewState), S.TerminalSideConnectionId, S.RelaySideConnectionId, S.TotalReadBytes, S.TotalWrittenBytes
    );

    return true;
}
