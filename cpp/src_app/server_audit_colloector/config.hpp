#pragma once
#include <pp_common/base.hpp>

struct xAuditControllerConfig : public xConfigLoader {

	void Load(const char * filename);

	xNetAddress TestAddress;
	std::string TestAppKey;
	std::string TestAppSecret;
};

extern xAuditControllerConfig GlobalConfig;