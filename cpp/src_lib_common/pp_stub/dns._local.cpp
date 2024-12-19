#include "./dns._local.hpp"

#include <chrono>

#if defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_DARWIN)
#include <arpa/inet.h>
#include <netdb.h>
#endif

bool xPPDnsServiceLocal::Init(size_t SubThreadNumber) {
	assert(SubThreads.empty());
	// just create sub threads as possible
	for (size_t i = 0; i < SubThreadNumber; ++i) {
		SubThreads.push_back(std::thread([this] { SubThreadLoop(); }));
	}
	if (SubThreads.empty()) {
		return false;
	}
	return true;
}

void xPPDnsServiceLocal::Clean() {
	RequestSemaphore.NotifyN(SubThreads.size());
	for (auto & T : SubThreads) {
		T.join();
	}
	// clean unprocessed request
	xel::RuntimeAssert(!RequestQueue.Pop());
	while (auto R = ResultQueue.Pop()) {
		X_PERROR("Unprocessed dns result: %s", R->Hostname.c_str());
		FreeResult(R);
	}
}

void xPPDnsServiceLocal::SubThreadLoop() {
	auto R = (xPPDnsQuery *)nullptr;
	while (true) {
		RequestSemaphore.Wait([&R, this] { R = RequestQueue.Pop(); });
		if (!R) {
			break;
		}
		{  // locally resolve dns:
			addrinfo   hints = {};
			addrinfo * res   = {};
			addrinfo * p     = {};

			memset(&hints, 0, sizeof(hints));
			hints.ai_family   = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;

			if (auto err = getaddrinfo(R->Hostname.c_str(), nullptr, &hints, &res)) {
				xel::Touch(err);
				X_PERROR("getaddrinfo: %s\n", gai_strerror(err));
			} else {
				for (p = res; p != NULL; p = p->ai_next) {
					if (p->ai_family == AF_INET) {  // IPv4
						if (R->A4) {
							continue;
						}
						R->A4 = xel::xNetAddress::Parse((sockaddr_in *)p->ai_addr);
					} else if (p->ai_family == AF_INET6) {
						if (R->A6) {
							continue;
						}
						R->A6 = xel::xNetAddress::Parse((sockaddr_in6 *)p->ai_addr);
					}
				}
				freeaddrinfo(res);
			}
		}
		{  // post result to resule queue
			auto L = xel::xSpinlockGuard(ResultQueueLock);
			ResultQueue.Push(*R);
		}
		xel::Todo("Wakeup request context");
	}
}

bool xPPDnsServiceLocal::PostDnsRequest(xel::xVariable UserContext, std::string_view Hostname) {
	try {
		auto R         = new xPPDnsQuery(Hostname);
		R->UserContext = UserContext;
		RequestSemaphore.Notify([&, this] { RequestQueue.Push(*R); });
	} catch (...) {
		return false;
	}
	return true;
}

xPPDnsResult * xPPDnsServiceLocal::GetNextResult() {
	auto L = xel::xSpinlockGuard(ResultQueueLock);
	return ResultQueue.Pop();
}

void xPPDnsServiceLocal::FreeResult(xPPDnsResult * R) {
	auto Node = static_cast<xPPDnsQuery *>(R);
	delete Node;
}
