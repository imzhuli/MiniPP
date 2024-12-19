#include "./geo_server.hpp"

bool xPP_GeoServer::Init(xIoContext * ICP, const xNetAddress & BindAddress) {
	RuntimeAssert(Server.Init(ICP, BindAddress, this));
	return true;
}

void xPP_GeoServer::Clean() {
	Server.Clean();
}
