#include "./config.hpp"

#include "./global.hpp"

#include <config/config.hpp>
#include <core/string.hpp>
#include <pp_common/region.hpp>
#include <unordered_map>

static std::string ForcedRelayerConfigFile;

std::vector<xCC_DeviceRelayServerInfoBase> ForceRelayServerList;

void LoadForcedRelayerServers() {
    if (ForcedRelayerConfigFile.empty()) {
        cout << "INFO: NO ForcedRelayerConfigFile" << endl;
        return;
    }
    cout << "INFO: Reading ForcedRelayers from " << ForcedRelayerConfigFile << endl;
    auto Contents = xel::FileToLines(ForcedRelayerConfigFile);
    for (auto & Line : Contents) {
        auto TL = xel::Split(Line, ",");
        cout << TL.size() << endl;
        for (auto & W : TL) {
            W = xel::Trim(W);
        }
        if (TL.empty() || TL[0][0] == '#') {
            cout << "INFO: Comment linne: " << Line << endl;
            continue;
        }
        if (TL.size() != 4) {
            cerr << "ERROR: Invalid format" << endl;
        }

        auto ServerId      = (uint32_t)atol(TL[0].c_str());
        auto CtrlAddress   = xNetAddress::Parse(TL[1]);
        auto DataAddress   = xNetAddress::Parse(TL[2]);
        auto RealCountryId = CountryCodeToCountryId(TL[3].c_str());

        RuntimeAssert(ServerId, "ServerId should be valid positive number");
        RuntimeAssert(CtrlAddress && CtrlAddress.Port, "CtrlAddress should be valid address with valid port");
        RuntimeAssert(DataAddress && DataAddress.Port, "CtrlAddress should be valid address with valid port");
        RuntimeAssert(RealCountryId, "RealCountryId should be valid two-digit string");

        auto Info                = xCC_DeviceRelayServerInfoBase();
        Info.ServerId            = ServerId;
        Info.RegionId.CountryId  = RealCountryId;
        Info.IsStaticRelayServer = true;
        Info.CtrlAddress         = CtrlAddress;
        Info.DataAddress         = DataAddress;
        ForceRelayServerList.push_back(Info);
    }
}

bool LoadConfig(const char * filename) {
    // LoadForced Relay
    auto Loader = xel::xConfigLoader(filename);

    Loader.Require(BindAddressForDevice, "BindAddressForDevice");
    Loader.Require(RelayDispatcherAddress, "RelayDispatcherAddress");

    Loader.Optional(ForcedRelayerConfigFile, "ForcedRelayerConfigFile");
    LoadForcedRelayerServers();

    return true;
}

/********************************
 *
 *   TEST
 *
 ********************************/

xCC_DeviceRelayServerInfoBase RelayForTest;
static auto                   InitTest = xInstantRun([] {
    RelayForTest.CtrlAddress = xNetAddress::Parse("192.168.5.112:17000");
    RelayForTest.DataAddress = xNetAddress::Parse("192.168.5.112:17001");
});
