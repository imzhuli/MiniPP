#include "./config.hpp"

xAuditControllerConfig GlobalConfig;

void xAuditControllerConfig::Load(const char * filename) {
	Reload(filename);
	Require(TestAddress, "TestAddress");
	Require(TestAppKey, "TestAppKey");
	Require(TestAppSecret, "TestAppSecret");
}
