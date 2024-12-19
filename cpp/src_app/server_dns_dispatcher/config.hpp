#pragma once
#include <pp_common/base.hpp>

class xGlobalConfig : public xConfigLoader {
public:
	void Load(const char * filename) {
		xConfigLoader::Reload(filename);
		Require(ServiceBindAddress, "service_bind_address");
		Require(ConsumerBindAddress, "consumer_bind_address");
		Require(ConfigCenterAddress, "config_center_address");
	}

public:
	xNetAddress ServiceBindAddress;
	xNetAddress ConsumerBindAddress;
	xNetAddress ConfigCenterAddress;
};

extern xGlobalConfig GlobalConfig;
