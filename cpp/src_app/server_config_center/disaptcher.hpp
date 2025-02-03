#pragma once
#include <pp_common/_.hpp>
#include <vector>

class xCC_BackendService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & DispatcherAddress);
    void Clean();

private:
    std::vector<xNetAddress> GeoServerList;
};

class xCC_DispatcherService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
    void Clean();

private:
};
