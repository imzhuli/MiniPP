#include "./_config.hpp"

#include "./_global.hpp"

#include <config/config.hpp>

size32_t MaxDeviceCount          = 10'0000;
size32_t MaxProxyCount           = 3000;
size32_t MaxRelayConnectionCount = 30'0000;

bool LoadConfig(const char * filename) {

    // LoadForced Relay
    auto Loader = xel::xConfigLoader(filename);

    Loader.Require(BindCtrlAddress, "BindCtrlAddress");
    Loader.Require(BindDataAddress, "BindDataAddress");
    Loader.Require(BindProxyAddress, "BindProxyAddress");

    return true;
}
