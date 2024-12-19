#pragma once
#include "./base.hpp"

#include <map>

class xHttpProxyClient : public xTcpConnection {

public:
	bool Init();
	void Clean();

protected:
	bool Connect();

private:
	enum eState {
		UNSPEC = 0,
		CONNECTING,
		AUTHENTICATING,

		// ERROR:
		DISCONNECTED = 1000,
		AUTH_FAILED,
		TARGET_CONNECTION_ABORT,
		TARGET_CLOSED,
	};

	std::string                        UserPass;
	std::map<std::string, std::string> ExtraHeaderFields;
};
