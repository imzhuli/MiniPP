#include "../common/challenge_client.hpp"
#include "./config.hpp"

#include <pp_common/base.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/config_center/dns_dispatcher.hpp>
#include <server_arch/service.hpp>

static constexpr const uint32_t DNS_DISPATCHER_VERSION = 0x01;
static constexpr const char *   DNS_DISPATCHER_KEY     = "Key40x01";

struct xDnsDispatcherServer : public xService {};
struct xConfigCenterClient : public xCClient {

	void PostChallengeRequest() override {
		X_DEBUG_PRINTF("");
		PrepareTimestampMS = GetTimestampMS();

		ubyte Buffer[MaxPacketSize];
		auto  PP               = xEnableDnsDispatcher{};
		PP.UnixTimestamp       = GetUnixTimestamp();
		PP.Version             = DNS_DISPATCHER_VERSION;
		PP.Challenge           = xEnableDnsDispatcher::GenerateChallenge(PP.UnixTimestamp, DNS_DISPATCHER_VERSION, DNS_DISPATCHER_KEY);
		PP.ServiceBindAddress  = GlobalConfig.ServiceBindAddress;
		PP.ConsumerBindAddress = GlobalConfig.ConsumerBindAddress;
		auto RSize             = WritePacket(Cmd_EnableDnsDispatcher, 0, Buffer, sizeof(Buffer), PP);
		PostData(Buffer, RSize);
	}

	bool OnChallengeResponse(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override {
		if (Header.CommandId != Cmd_EnableDnsDispatcherResp) {
			return false;
		}
		auto Resp = xEnableDnsDispatcherResp();
		if (PayloadSize != Resp.Deserialize(PayloadPtr, PayloadSize) || !Resp.Accepted) {
			X_DEBUG_PRINTF("Not Accepted");
			return false;
		}
		X_DEBUG_PRINTF("Accepted");
		return true;
	}

	bool OnBusinessPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override {
		return true;
	}

private:
	uint64_t PrepareTimestampMS = 0;
};

static xIoContext           IC;
static xDnsDispatcherServer Server;
static xConfigCenterClient  CCClient;

int main(int argc, char ** argv) {

	auto CL = xCommandLine(
		argc, argv,
		{
			{ 'c', nullptr, "config", true },
		}
	);
	auto C = CL["config"];
	if (!C()) {
		cerr << "missing config filename" << endl;
		X_PFATAL("missing config filename: param -c config_filename");
	}

	GlobalConfig.Load(C->c_str());

	auto ICG = xResourceGuard(IC);
	auto SG  = xResourceGuard(Server, &IC, GlobalConfig.ConsumerBindAddress);
	if (!SG) {
		X_PFATAL("failed to bind address");
	}
	auto CG = xResourceGuard(CCClient, &IC, GlobalConfig.ConfigCenterAddress);

	auto Ticker = xTicker();
	while (true) {
		Ticker.Update();
		IC.LoopOnce();
		Server.Tick(Ticker());
		CCClient.Tick(Ticker());
	}

	return 0;
}
