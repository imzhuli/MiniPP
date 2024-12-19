#include "./config.hpp"
#include "./global.hpp"
#include "./relay_service.hpp"

#include <pp_protocol/relay_terminal/init_ctrl_stream.hpp>

int main(int argc, char ** argv) {

	auto CL = xCommandLine(
		argc, argv,
		{ {
			'c',
			nullptr,
			"config_file",
			true,
		} }
	);

	auto ConfigFileOpt = CL["config_file"];
	RuntimeAssert(ConfigFileOpt());
	RuntimeAssert(LoadConfig(ConfigFileOpt->c_str()));

	RuntimeAssert(GlobalIoContext.Init());
	RuntimeAssert(DeviceManager.Init(MaxDeviceCount));
	RuntimeAssert(ConnectionManager.Init(&GlobalIoContext, MaxDeviceCount * 2));
	RuntimeAssert(DeviceRelayService.Init(&GlobalIoContext, BindControlAddress, BindDataAddress));

	while (true) {
		auto NowMS = GetTimestampMS();
		GlobalIoContext.LoopOnce();
		ConnectionManager.Tick(NowMS);
		DeviceManager.Tick(NowMS);
		DeviceRelayService.Tick(NowMS);
	}

	DeviceRelayService.Clean();
	ConnectionManager.Clean();
	DeviceManager.Clean();
	GlobalIoContext.Clean();

	return 0;
}
