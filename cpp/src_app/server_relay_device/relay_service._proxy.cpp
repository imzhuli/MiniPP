#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>
#include <pp_protocol/relay_terminal/connection.hpp>

bool xDeviceRelayService::OnProxyPacket(xRD_ProxyConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    switch (Header.CommandId) {
        case Cmd_PA_RL_Challenge:
            return OnProxyChallenge(Conn, Header, Payload, PayloadSize);
        case Cmd_PA_RL_CreateConnection:
            return OnProxyCreateConnection(Conn, Header, Payload, PayloadSize);
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

    auto CC                  = xTR_CreateConnection();
    CC.RelaySideConnectionId = 12345;
    CC.TargetAddress         = R.TargetAddress;
    D->CtrlConnection->PostPacket(Cmd_Terminal_RL_CreateConnection, 0, CC);

    return true;
}
