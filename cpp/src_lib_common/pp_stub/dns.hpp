#pragma once
#include <core/core_min.hpp>
#include <network/net_address.hpp>

struct xPPDnsResult;

class xPPDnsResultIterator;
class iPPDnsServiceStub;

struct xPPDnsResult {
	xel::xVariable   UserContext;
	xel::xNetAddress A4;
	xel::xNetAddress A6;
};

/**
	xPPDnsResultIterator:
		objects that helps calling peek & release dns result from iPPDnsServiceStub

		xPPDnsResultIterator():
			bind iterator with dns service;
		Next():
			release current result (if any) and get a next result
		~xPPDnsResultIterator():
			release current result (if any)

	example:
		iPPDnsServiceStub * Stub;
		...
		auto Iter = Stub.GetResultIterator();
		while(auto Result = Iter.Next()) {
			ProcessResult(Result);
		}
 */
class xPPDnsResultIterator final : xel::xNonCopyable {
private:
	friend class iPPDnsServiceStub;
	xPPDnsResultIterator(iPPDnsServiceStub * Stub);

public:
	xPPDnsResult * Next();
	~xPPDnsResultIterator();

private:
	iPPDnsServiceStub * Stub          = nullptr;
	xPPDnsResult *      CurrentResult = nullptr;
};

class iPPDnsServiceStub : xel::xAbstract {
	friend class xPPDnsResultIterator;

public:
	virtual bool                  PostDnsRequest(xel::xVariable UserContext, std::string_view Hostname) = 0;
	X_INLINE xPPDnsResultIterator GetResultIterator() {
		return xPPDnsResultIterator(this);
	}

private:  // called by iterator
	virtual xPPDnsResult * GetNextResult()            = 0;
	virtual void           FreeResult(xPPDnsResult *) = 0;
};
