#include "./device_relay_manager.hpp"

using namespace xel;

static xCC_DeviceRelayServerInfoList TimeoutDeviceRelayServerList;

bool AddRelayServer(const xCC_DeviceRelayServerInfoBase & BaseInfo) {
	return false;
}

void UpdateRelayServer(uint32_t ServerId) {
}

void RemoveTimeoutRelayServers() {
	Pure();
}
