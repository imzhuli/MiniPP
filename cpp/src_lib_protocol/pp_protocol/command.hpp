#pragma once
#include <pp_common/base.hpp>

static constexpr const xPacketCommandId CmdRespBase = MaxDispatchableCommandIdCount;

static constexpr const xPacketCommandId Cmd_GeoQuery            = 0x01;
static constexpr const xPacketCommandId Cmd_HostQuery           = 0x02;
static constexpr const xPacketCommandId Cmd_GetTerminalWithAuth = 0x03;
static constexpr const xPacketCommandId Cmd_ReportAccountUsage  = 0x04;
static constexpr const xPacketCommandId Cmd_ProxyClientAuth     = 0x05;

static constexpr const xPacketCommandId Cmd_GeoQueryResp            = CmdRespBase + Cmd_GeoQuery;
static constexpr const xPacketCommandId Cmd_HostQueryResp           = CmdRespBase + Cmd_HostQuery;
static constexpr const xPacketCommandId Cmd_GetTerminalWithAuthResp = CmdRespBase + Cmd_GetTerminalWithAuth;
static constexpr const xPacketCommandId Cmd_ReportAccountUsageResp  = CmdRespBase + Cmd_ReportAccountUsage;  // not used
static constexpr const xPacketCommandId Cmd_ProxyClientAuthResp     = CmdRespBase + Cmd_ProxyClientAuth;

// non dispatchable or direct command
static constexpr const xPacketCommandId CmdProxyBase                 = 0x02'000;
static constexpr const xPacketCommandId Cmd_CreateConnection         = CmdProxyBase + 0x01;
static constexpr const xPacketCommandId Cmd_CreateConnectionResp     = CmdProxyBase + 0x02;
static constexpr const xPacketCommandId Cmd_CloseConnection          = CmdProxyBase + 0x03;
static constexpr const xPacketCommandId Cmd_PostProxyToRelayData     = CmdProxyBase + 0x04;
static constexpr const xPacketCommandId Cmd_PostRelayToProxyData     = CmdProxyBase + 0x05;
static constexpr const xPacketCommandId Cmd_CreateUdpAssociation     = CmdProxyBase + 0x06;
static constexpr const xPacketCommandId Cmd_CreateUdpAssociationResp = CmdProxyBase + 0x07;
static constexpr const xPacketCommandId Cmd_CloseUdpAssociation      = CmdProxyBase + 0x08;
static constexpr const xPacketCommandId Cmd_PostProxyToRelayUdpData  = CmdProxyBase + 0x09;
static constexpr const xPacketCommandId Cmd_PostRelayToProxyUdpData  = CmdProxyBase + 0x0A;

// management
static constexpr const xPacketCommandId Cmd_ManagementBase       = 0x02'100;
static constexpr const xPacketCommandId Cmd_PostError            = Cmd_ManagementBase + 0x01;
static constexpr const xPacketCommandId Cmd_PostErrorResp        = Cmd_ManagementBase + 0x02;
static constexpr const xPacketCommandId Cmd_PostTerminalAudit    = Cmd_ManagementBase + 0x03;
static constexpr const xPacketCommandId Cmd_PostRelayServerAudit = Cmd_ManagementBase + 0x04;

// version 2024 from here

// backend base:
static constexpr const xPacketCommandId Cmd_BackendBase             = 0x04'000;
static constexpr const xPacketCommandId Cmd_AuditTerminalInfo       = Cmd_BackendBase + 0x01;
static constexpr const xPacketCommandId Cmd_AuditTerminalInfoResp   = Cmd_BackendBase + 0x02;  // 一般不处理返回
static constexpr const xPacketCommandId Cmd_AuditUsageByAuditId     = Cmd_BackendBase + 0x03;
static constexpr const xPacketCommandId Cmd_AuditUsageByAuditIdResp = Cmd_BackendBase + 0x04;  // 一般不处理返回
static constexpr const xPacketCommandId Cmd_BackendChallenge        = Cmd_BackendBase + 0x101;
static constexpr const xPacketCommandId Cmd_BackendChallengeResp    = Cmd_BackendBase + 0x102;
static constexpr const xPacketCommandId Cmd_AuthByUserPass          = Cmd_BackendBase + 0x201;
static constexpr const xPacketCommandId Cmd_AuthByUserPassResp      = Cmd_BackendBase + 0x202;

// server management:
// clang-format on
static constexpr const xPacketCommandId Cmd_ServerManagementBase    = 0x05'000;
static constexpr const xPacketCommandId Cmd_QueryRunningState       = Cmd_ServerManagementBase + 1;
static constexpr const xPacketCommandId Cmd_QueryRunningStateResp   = Cmd_ServerManagementBase + 2;
static constexpr const xPacketCommandId Cmd_RegisterDnsServer       = Cmd_ServerManagementBase + 3;  // get dns server group id from config center
static constexpr const xPacketCommandId Cmd_RegisterDnsServerResp   = Cmd_ServerManagementBase + 4;
static constexpr const xPacketCommandId Cmd_CheckinDnsServer        = Cmd_ServerManagementBase + 5;  // check into dispatcher with group id from config center
static constexpr const xPacketCommandId Cmd_CheckinDnsServerResp    = Cmd_ServerManagementBase + 6;
static constexpr const xPacketCommandId Cmd_EnableDnsDispatcher     = Cmd_ServerManagementBase + 7;
static constexpr const xPacketCommandId Cmd_EnableDnsDispatcherResp = Cmd_ServerManagementBase + 8;

static constexpr const xPacketCommandId Cmd_GetBackendServerList     = Cmd_ServerManagementBase + 100;
static constexpr const xPacketCommandId Cmd_GetBackendServerListResp = Cmd_ServerManagementBase + 101;
// clang-formt on

// proxyaccess-relay:
static constexpr const xPacketCommandId Cmd_PA_RL_Base                  = 0x06'000;
static constexpr const xPacketCommandId Cmd_PA_RL_Challenge             = Cmd_PA_RL_Base + 0x01;
static constexpr const xPacketCommandId Cmd_PA_RL_ChallengeResp         = Cmd_PA_RL_Base + 0x02;
static constexpr const xPacketCommandId Cmd_PA_RL_CreateConnection      = Cmd_PA_RL_Base + 0x03;
static constexpr const xPacketCommandId Cmd_PA_RL_DestroyConnection     = Cmd_PA_RL_Base + 0x04;
static constexpr const xPacketCommandId Cmd_PA_RL_NotifyConnectionState = Cmd_PA_RL_Base + 0x05;
static constexpr const xPacketCommandId Cmd_PA_RL_PostData              = Cmd_PA_RL_Base + 0x06;
static constexpr const xPacketCommandId Cmd_PA_RL_ProxyClientNotify     = Cmd_PA_RL_Base + 0x09;

// terminal-config-center:
static constexpr const xPacketCommandId Cmd_Terminal_CC_Base          = 0x07'000;
static constexpr const xPacketCommandId Cmd_Terminal_CC_Challenge     = Cmd_Terminal_CC_Base + 0x01;
static constexpr const xPacketCommandId Cmd_Terminal_CC_ChallengeResp = Cmd_Terminal_CC_Base + 0x02;

// terminal-relay:
static constexpr const xPacketCommandId Cmd_Terminal_RL_Base                  = 0x08'000;
static constexpr const xPacketCommandId Cmd_Terminal_RL_InitCtrlStream        = Cmd_Terminal_RL_Base + 0x01;
static constexpr const xPacketCommandId Cmd_Terminal_RL_InitCtrlStreamResp    = Cmd_Terminal_RL_Base + 0x02;
static constexpr const xPacketCommandId Cmd_Terminal_RL_InitDataStream        = Cmd_Terminal_RL_Base + 0x03;
static constexpr const xPacketCommandId Cmd_Terminal_RL_InitDataStreamResp    = Cmd_Terminal_RL_Base + 0x04;
static constexpr const xPacketCommandId Cmd_Terminal_RL_CreateConnection      = Cmd_Terminal_RL_Base + 0x05;
static constexpr const xPacketCommandId Cmd_Terminal_RL_DestroyConnection     = Cmd_Terminal_RL_Base + 0x06;
static constexpr const xPacketCommandId Cmd_Terminal_RL_NotifyConnectionState = Cmd_Terminal_RL_Base + 0x07;
static constexpr const xPacketCommandId Cmd_Terminal_RL_PostData              = Cmd_Terminal_RL_Base + 0x08;
static constexpr const xPacketCommandId Cmd_Terminal_RL_ProxyClientNotify     = Cmd_Terminal_RL_Base + 0x09;
static constexpr const xPacketCommandId Cmd_Terminal_RL_ForceDetroyConnection = Cmd_Terminal_RL_Base + 0x0A;

static constexpr const xPacketCommandId Cmd_Terminal_RL_CreateUdpChannel     = Cmd_Terminal_RL_Base + 0x10;
static constexpr const xPacketCommandId Cmd_Terminal_RL_CloseUdpChannel      = Cmd_Terminal_RL_Base + 0x11;
static constexpr const xPacketCommandId Cmd_Terminal_RL_PostUdpChannelData   = Cmd_Terminal_RL_Base + 0x12;
static constexpr const xPacketCommandId Cmd_Terminal_RL_CreateUdpChannelResp = Cmd_Terminal_RL_Base + 0x13;
static constexpr const xPacketCommandId Cmd_Terminal_RL_UdpChannelKeepAlive  = Cmd_Terminal_RL_Base + 0x14;

static constexpr const xPacketCommandId Cmd_Terminal_RL_RequireLog = Cmd_Terminal_RL_Base + 0x20;
static constexpr const xPacketCommandId Cmd_Terminal_RL_PushLog    = Cmd_Terminal_RL_Base + 0x21;

std::vector<ubyte> Encrypt(const void * Data, size_t DataSize, const std::string & AesKey);
std::vector<ubyte> Decrypt(const void * Data, size_t DataSize, const std::string & AesKey);
