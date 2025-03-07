#pragma once
#include <pp_common/request.hpp>
#include <pp_component/dns/dns_service.hpp>

struct xDnsRequestSource {
	uint64_t RequestId;
};

struct xDnsRequest : xDnsRequestSource {
	std::string Hostname;
};

using xDnsRequestManager = xAsyncRequestManager<xDnsRequestSource, xDnsResult>;

class xDnsServer
	: public xDnsRequestManager
	, public xClient {
public:
	bool Init(xNetAddress DispatcherAddress, size_t MaxRequestCount);
	void Clean();
	void Tick();

private:
	xIoContext  IoContext;
	xTicker     Ticker;
	xDnsService LocalDnsService;

private:
	void OnServerConnected() override;
	bool OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;
	void PostRequest(uint64_t RequestId, const xRequestSource & Source) override;
	void PostResult(const xRequestSource & Source, bool HasData, const xDataNode & Data) override;

private:
	static void OnLocalDnsWakeup(xVariable Ctx);
	static void OnLocalDnsResult(xVariable CallbackCtx, xVariable RequestCtx, const xNetAddress & A4, const xNetAddress & A6);
};
