#include "./config.hpp"

#include "./global.hpp"

#include <config/config.hpp>

size32_t MaxDeviceCount = 3000;

bool LoadConfig(const char * filename) {

	// LoadForced Relay
	auto Loader = xel::xConfigLoader(filename);

	Loader.Require(BindControlAddress, "BindControlAddress");
	Loader.Require(BindDataAddress, "BindDataAddress");
	Loader.Require(BindProxyAddress, "BindProxyAddress");

	return true;
}
