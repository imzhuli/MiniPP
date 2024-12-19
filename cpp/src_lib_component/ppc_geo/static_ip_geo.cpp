#include "./static_ip_geo.hpp"

bool xStaticIPManager::Init() {
	RuntimeAssert(IPMap.empty());
	return true;
}

void xStaticIPManager::Clean() {
	IPMap.clear();
}

void xStaticIPManager::Reload(const char * filename) {
	Pure();
}

const xPP_StaticIPGeo * xStaticIPManager::Query(const xNetAddress & IP) {
	auto Iter = IPMap.find(IP);
	if (Iter == IPMap.end()) {
		return nullptr;
	}
	return &Iter->second;
}