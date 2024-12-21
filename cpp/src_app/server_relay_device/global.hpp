#pragma once
#include "./connection_manager.hpp"
#include "./device_manager.hpp"
#include "./relay_service.hpp"

#include <pp_common/_.hpp>

extern xIoContext  GlobalIoContext;
extern xNetAddress BindControlAddress;
extern xNetAddress BindDataAddress;
extern xNetAddress BindProxyAddress;

extern xRL_DeviceConnectionManager ConnectionManager;
extern xDeviceManager              DeviceManager;
extern xDeviceRelayService         DeviceRelayService;
