#include "./dns.hpp"

xPPDnsResultIterator::xPPDnsResultIterator(iPPDnsServiceStub * Stub)
	: Stub(Stub) {
}

xPPDnsResult * xPPDnsResultIterator::Next() {
	if (CurrentResult) {
		Stub->FreeResult(CurrentResult);
	}
	return (CurrentResult = Stub->GetNextResult());
}

xPPDnsResultIterator::~xPPDnsResultIterator() {
	if (CurrentResult) {
		Stub->FreeResult(CurrentResult);
	}
}
