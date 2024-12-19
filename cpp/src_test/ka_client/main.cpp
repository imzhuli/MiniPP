#include <ppc_common/client.hpp>

auto IC              = xIoContext();
auto RIC             = xResourceGuard(IC);
auto TargetAddresses = std::vector<xNetAddress>{
	xNetAddress::Parse("0.0.0.0:7777"),
	xNetAddress::Parse("0.0.0.0:7777"),
	xNetAddress::Parse("0.0.0.0:7777"),
};
auto MyClient = xPP_Client();

int main(int argc, char ** argv) {

	RuntimeAssert(MyClient.Init(&IC));
	MyClient.UpdateServerAddresses(TargetAddresses);

	while (true) {
		IC.LoopOnce();
		MyClient.Tick();
	}
	return 0;
}
