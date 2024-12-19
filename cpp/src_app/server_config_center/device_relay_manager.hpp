#pragma once
#include "./typedef.hpp"

bool AddRelayServer(const xCC_DeviceRelayServerInfoBase & BaseInfo);
void UpdateRelayServer(uint32_t ServerId);
void RemoveTimeoutRelayServers();
