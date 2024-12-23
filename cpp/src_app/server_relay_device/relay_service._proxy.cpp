#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

bool xDeviceRelayService::OnProxyPacket(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    switch (Header.CommandId) {
        case Cmd_PA_RL_Challenge:
            return OnProxyChallenge(Conn, Header, Payload, PayloadSize);
        case Cmd_PA_RL_CreateConnection:
            return OnProxyCreateConnection(Conn, Header, Payload, PayloadSize);
        case Cmd_PA_RL_DestroyConnection:
            return OnProxyDestroyConnection(Conn, Header, Payload, PayloadSize);
        case Cmd_PA_RL_PostData:
            return OnProxyPushData(Conn, Header, Payload, PayloadSize);
        default:
            X_DEBUG_PRINTF("unrecognized protocol %" PRIx32 "", Header.CommandId);
            break;
    }
    return false;
}

bool xDeviceRelayService::OnProxyChallenge(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto R = xPR_Challenge();
    if (!R.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    X_DEBUG_PRINTF("");
    Conn->SetChallengeReady();
    ProxyConnectionManager.KeepAlive(Conn);

    auto Resp     = xPR_ChallengeResp();
    Resp.Accepted = true;
    Conn->PostPacket(Cmd_PA_RL_ChallengeResp, Header.RequestId, Resp);

    return true;
}

bool xDeviceRelayService::OnProxyCreateConnection(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto R = xPR_CreateConnection();
    if (!R.Deserialize(Payload, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF("NewConnection: did=%" PRIx64 ", ProxySideConnectionId=%" PRIx64 "", R.RelaySideDeviceId, R.ProxySideConnectionId);

    auto D = DeviceManager.GetDeviceById(R.RelaySideDeviceId);
    if (!D) {
        X_DEBUG_PRINTF("Device not found");
        return true;
    }
    assert(D->CtrlConnection);

    auto RCC = RelayConnectionManager.Create();
    if (!RCC) {
        auto F                  = xPR_ConnectionStateNotify();
        F.NewState              = xPR_ConnectionStateNotify::STATE_CLOSED;
        F.ProxySideConnectionId = R.ProxySideConnectionId;
        Conn->PostPacket(Cmd_PA_RL_NotifyConnectionState, 0, F);
        return true;
    }
    X_DEBUG_PRINTF("RelaySideConnectionId=%" PRIx64 "", RCC->RelaySideConnectionId);

    RCC->DeviceId              = D->DeviceRuntimeId;
    RCC->ProxyId               = Conn->ConnectionId;
    RCC->ProxySideConnectionId = R.ProxySideConnectionId;

    auto CC                  = xTR_CreateConnection();
    CC.RelaySideConnectionId = RCC->RelaySideConnectionId;
    CC.TargetAddress         = R.TargetAddress;
    D->CtrlConnection->PostPacket(Cmd_Terminal_RL_CreateConnection, 0, CC);

    return true;
}

bool xDeviceRelayService::OnProxyDestroyConnection(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto R = xPR_DestroyConnection();
    if (!R.Deserialize(Payload, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF("RelaySizeConnectionId=%" PRIx64 "", R.RelaySideConnectionId);
    auto RC = RelayConnectionManager.GetConnectionById(R.RelaySideConnectionId);
    if (!RC) {
        X_DEBUG_PRINTF("relay connection not found");
        return true;
    }

    auto RL                   = xTR_DestroyConnection();
    RL.DeviceSideConnectionId = RC->DeviceSideConnectionId;
    RL.RelaySideConnectionId  = RC->RelaySideConnectionId;

    auto D = DeviceManager.GetDeviceById(RC->DeviceId);
    if (!D) {
        X_DEBUG_PRINTF("Device not found");
        return true;
    }
    assert(D->DataConnection);

    D->DataConnection->PostPacket(Cmd_Terminal_RL_DestroyConnection, 0, RL);
    return true;
}

bool xDeviceRelayService::OnProxyPushData(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("");
    auto R = xPR_PushData();
    if (!R.Deserialize(Payload, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF(
        "RelaySideConnectionId=%" PRIx64 ", ProxySideConnectionId=%" PRIx64 ", Data=\n%s", R.RelaySideConnectionId, R.ProxySideConnectionId,
        HexShow(R.PayloadView).c_str()
    );

    auto RC = RelayConnectionManager.GetConnectionById(R.RelaySideConnectionId);
    if (!RC) {
        X_DEBUG_PRINTF("Connection not found");
        return true;
    }
    if (RC->ProxySideConnectionId != R.ProxySideConnectionId) {
        X_DEBUG_PRINTF("Proxy side connection id not match");
        return true;
    }

    auto DC = DeviceManager.GetDeviceById(RC->DeviceId);
    if (!DC) {
        X_DEBUG_PRINTF("Device connection not found");
        return true;
    }

    auto Push                   = xTR_PostData();
    Push.DeviceSideConnectionId = RC->DeviceSideConnectionId;
    Push.RelaySideConnectionId  = RC->RelaySideConnectionId;
    Push.PayloadView            = R.PayloadView;

    DC->DataConnection->PostPacket(Cmd_Terminal_RL_PostData, 0, Push);
    return true;
}