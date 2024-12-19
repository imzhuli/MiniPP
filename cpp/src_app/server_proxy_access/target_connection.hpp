#pragma once
#include <core/core_min.hpp>

enum xPA_Flags : uint32_t {
	PAF_CLEAR                 = 0,
	PAF_SOCKS5_TCP_CONNECTION = 0x01,
	PAF_SOCKS5_UDP_HOLDER     = 0x02,
	PAF_SOCKS5_UDP_CHANNEL    = 0x03,
};

struct xPA_TargetConnectionContext {
	uint64_t LocalConnectionId;
	uint64_t RelaySideConnectionId;
	uint64_t RelayConnectionId;

	uint64_t IdleTimestampMS;
};
