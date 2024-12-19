#include "./challenge_client.hpp"

#include <core/core_time.hpp>

using namespace xel;

bool xCClient::Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress) {
	assert(!Ready);
	RuntimeAssert(xClient::Init(ICP, TargetAddress));
	return true;
}

void xCClient::Clean() {
	xClient::Clean();
	Reset(Connected);
	Reset(Ready);
}

void xCClient::Tick(uint64_t NowMS) {
	if (Connected && !Ready && NowMS - LastConnectedTimestamp >= 3'000) {
		Kill();
	}
	xClient::Tick(NowMS);
}

void xCClient::OnServerConnected() {
	X_DEBUG_PRINTF("");
	assert(!Ready);
	Connected              = true;
	LastConnectedTimestamp = GetTimestampMS();
	PostChallengeRequest();
}

void xCClient::OnServerClose() {
}

void xCClient::OnOpenConnection() {
	X_DEBUG_PRINTF("");
}

void xCClient::OnCleanupConnection() {
	X_DEBUG_PRINTF("");
	LastConnectedTimestamp = 0;
	Connected              = false;
	Ready                  = false;
	OnCCClose();
}

bool xCClient::OnPacket(const xel::xPacketHeader & Header, xel::ubyte * PayloadPtr, size_t PayloadSize) {
	// X_DEBUG_PRINTF("");
	if (Ready) {
		return (Ready = OnBusinessPacket(Header, PayloadPtr, PayloadSize));
	}
	if ((Ready = OnChallengeResponse(Header, PayloadPtr, PayloadSize))) {
		OnCCReady();
	}
	return true;
}
