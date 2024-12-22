#include "./device_manager.hpp"

#include "./_global.hpp"

bool xDeviceManager::Init(size_t MaxDevice) {
    if (!DevicePool.Init(MaxDevice)) {
        return false;
    }
    auto DPC = MakeResourceCleaner(DevicePool);

    DPC.Dismiss();
    return true;
}

void xDeviceManager::Clean() {
    // build kill list

    // destroy any device context in kill list
    while (auto DC = DeviceKillList.PopHead()) {
        DestroyDevice(DC);
    }
    auto DPC = MakeResourceCleaner(DevicePool);
}

void xDeviceManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
}

auto xDeviceManager::CreateDevice() -> xDevice * {
    auto ContextId = DevicePool.Acquire();
    if (!ContextId) {
        return nullptr;
    }
    auto & ContextRef          = DevicePool[ContextId];
    ContextRef.DeviceRuntimeId = ContextId;
    return &ContextRef;
}

void xDeviceManager::DestroyDevice(xDevice * DC) {
    DevicePool.Release(DC->DeviceRuntimeId);
}

void xDeviceManager::ReportDeviceState(xDevice * Device) {
    Todo("");
}
//
