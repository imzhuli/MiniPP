#pragma once
#include <pp_common/_.hpp>

class xRD_DeviceConnection;

struct xDevice : xListNode {
    static constexpr uint32_t FLAG_INIT   = 0x00;
    static constexpr uint32_t FLAG_BOUND  = 0x01;
    static constexpr uint32_t FLAG_DELETE = 0x80;

    xIndexId               DeviceRuntimeId = 0;
    uint32_t               Flags           = FLAG_INIT;
    xRD_DeviceConnection * CtrlConnection  = 0;
    xRD_DeviceConnection * DataConnection  = 0;

    void MarkDelete() {
        Flags |= FLAG_DELETE;
    }
    bool IsBeingDeleted() const {
        return Flags & FLAG_DELETE;
    }
};

class xDeviceManager {

public:
    bool Init(size_t MaxDevice);
    void Clean();
    void Tick(uint64_t NowMS);

    xDevice * GetDeviceById(uint64_t DeviceId) {
        auto Device = DevicePool.CheckAndGet(DeviceId);
        if (!Device || Device->IsBeingDeleted()) {
            return nullptr;
        }
        return Device;
    }

public:
    auto NewDevice() -> xDevice * {
        return CreateDevice();
    }
    void ReleaseDevice(xDevice * DC) {
        DeferDestroyDevice(DC);
    }

protected:
    auto CreateDevice() -> xDevice *;
    void DeferDestroyDevice(xDevice * DC) {
        DC->MarkDelete();
        DeviceKillList.GrabTail(*DC);
    }
    void DestroyDevice(xDevice * DC);
    void ReportDeviceState(xDevice * Device);

private:
    xTicker                  Ticker;
    xIndexedStorage<xDevice> DevicePool;
    xList<xDevice>           DeviceKillList;
    //
};
