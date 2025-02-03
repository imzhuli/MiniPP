#include <core/core_time.hpp>
#include <pp_common/base.hpp>

// dont reorder
#include "./config.hpp"
#include "./global.hpp"
#include "./typedef.hpp"

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', nullptr, "config_file", true },
            { 'd', nullptr, "dispatcher", false },
        }
    );

    auto ConfigFileOpt = CL["config_file"];
    RuntimeAssert(ConfigFileOpt());

    LoadConfig(ConfigFileOpt->c_str());

    RuntimeAssert(GlobalIoContext.Init());
    RuntimeAssert(InitDeviceChallengeManager());

    while (true) {
        GlobalIoContext.LoopOnce();
    }

    CleanDeviceChallengeManager();
    GlobalIoContext.Clean();

    return 0;
}
