#pragma once
#include <server_arch/client.hpp>

class xCClient : private xel::xClient {

public:
	bool Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress);
	void Clean();
	void Tick(uint64_t NowMS);

	bool IsReady() const {
		return Ready;
	}

protected:
	using xClient::PostData;
	using xClient::SetMaxWriteBuffer;
	virtual void PostChallengeRequest()                                                                              = 0;
	virtual bool OnChallengeResponse(const xel::xPacketHeader & Header, xel::ubyte * PayloadPtr, size_t PayloadSize) = 0;
	virtual bool OnBusinessPacket(const xel::xPacketHeader & Header, xel::ubyte * PayloadPtr, size_t PayloadSize)    = 0;
	virtual void OnCCReady() {};
	virtual void OnCCClose() {};

private:
	void OnServerConnected() override final;
	void OnServerClose() override final;
	bool OnPacket(const xel::xPacketHeader & Header, xel::ubyte * PayloadPtr, size_t PayloadSize) override final;
	void OnOpenConnection() override final;
	void OnCleanupConnection() override final;

private:
	uint64_t LastConnectedTimestamp = 0;
	bool     Connected              = false;
	bool     Ready                  = false;
};
