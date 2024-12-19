#pragma once
#include "./challenge_client.hpp"

class xBackendClient : public xCClient {
public:
	bool Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress);
	using xCClient::Clean;

	void PostChallengeRequest() override;
	bool OnChallengeResponse(const xel::xPacketHeader & Header, xel::ubyte * PayloadPtr, size_t PayloadSize) override;

private:
	uint64_t PrepareTimestampMS = 0;
};
