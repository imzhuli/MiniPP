#pragma once
#include "../ppc_common/client.hpp"

class xPP_GeoClient : public xPP_Client {
public:
	using xPP_Client::Clean;
	using xPP_Client::Init;

	bool MakeRequest(const xNetAddress & Address);

	bool OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;
	void Tick();

private:
	struct xRequestContext : xListNode {
		xIndexId LocalRequestId;
		xIndexId SourceRequestId;
		uint64_t TimestampMS;
	};

	xIndexedStorage<xRequestContext> RequestManager;
	xList<xRequestContext>           RequestTimeoutList;
};
