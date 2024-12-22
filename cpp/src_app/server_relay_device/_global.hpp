#pragma once
#include "./device_manager.hpp"
#include "./relay_connection_manager.hpp"
#include "./relay_service.hpp"

#include <pp_common/_.hpp>

extern xIoContext  GlobalIoContext;
extern xNetAddress BindControlAddress;
extern xNetAddress BindDataAddress;
extern xNetAddress BindProxyAddress;

extern xRD_DeviceConnectionManager DeviceConnectionManager;
extern xDeviceManager              DeviceManager;
extern xRD_ProxyConnectionManager  ProxyConnectionManager;
extern xDeviceRelayService         DeviceRelayService;
extern xRelayConnectionManager     RelayConnectionManager;
