#include <pp_common/_.hpp>
#include <server_arch/service.hpp>

auto IC          = xIoContext();
auto RIC         = xResourceGuard(IC);
auto BindAddress = xNetAddress::Parse("0.0.0.0:7777");

struct xMyServer : xel::xService {};
auto MyServer = xMyServer();

int main(int argc, char ** argv) {

	RuntimeAssert(MyServer.Init(&IC, BindAddress, true));

	while (true) {
		IC.LoopOnce();
		MyServer.Tick();
	}

	return 0;
}
