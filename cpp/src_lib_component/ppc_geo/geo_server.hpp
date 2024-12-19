#pragma once
#include <maxminddb.h>

#include <pp_common/_.hpp>

class xPP_GeoServer : public xTcpServer::iListener {
public:
	bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
	void Clean();

	//
protected:
	xTcpServer Server;
};
