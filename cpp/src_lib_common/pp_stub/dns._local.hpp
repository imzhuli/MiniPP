#include "./dns.hpp"

#include <core/queue.hpp>
#include <core/thread.hpp>
#include <string>
#include <thread>
#include <vector>

class xPPDnsServiceLocal : public iPPDnsServiceStub {
private:
	struct xPPDnsQuery
		: xel::xQueueNode
		, xPPDnsResult {
		std::string Hostname;

		xPPDnsQuery(std::string_view HostnameView)
			: Hostname(std::string(HostnameView)) {
		}
	};

public:
	bool Init(size_t SubThreadNumber);
	void Clean();
	bool PostDnsRequest(xel::xVariable UserContext, std::string_view Hostname) override;

private:
	void SubThreadLoop();

	xPPDnsResult * GetNextResult() override;
	void           FreeResult(xPPDnsResult *) override;

private:
	std::vector<std::thread> SubThreads;
	xel::xSemaphore          RequestSemaphore;
	xel::xQueue<xPPDnsQuery> RequestQueue;
	xel::xSpinlock           ResultQueueLock;
	xel::xQueue<xPPDnsQuery> ResultQueue;
};
