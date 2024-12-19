#include <pp_common/_.hpp>
//
#include <core/timer_wheel.hpp>
#include <iostream>
#include <network/net_address.hpp>
#include <pp_common/hostname.hpp>
#include <pp_protocol/config_center/dns_dispatcher.hpp>
#include <pp_protocol/dns_server/dns_dispatcher.hpp>

using namespace std;
using namespace xel;

int main(int, char **) {
	xIndexedStorage<int> IS;
	IS.Init(1024);

	auto Id = IS.Acquire(0);
	IS.Release(Id);
	RuntimeAssert(!IS.Check(Id));

	return 0;
}
