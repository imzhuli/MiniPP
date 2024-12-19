#include "./hostname.hpp"

uint32_t HashHostname(const char * Hostname) {
	uint32_t H = 0;
	while (auto C = *(Hostname++)) {
		H = H * 33 + C;
	}
	return H;
}

uint32_t HashHostname(const char * Hostname, size_t Len) {
	uint32_t H = 0;
	for (size_t I = 0; I < Len; ++I) {
		H = H * 33 + Hostname[I];
	}
	return H;
}

uint32_t HashHostname(const std::string & Hostname) {
	return HashHostname(Hostname.data(), Hostname.size());
}
