#include <pp_common/base.hpp>
#include <pp_protocol/backend/backend_challenge.hpp>

class xAuditReportConnection : xel::xTcpConnection::iListener {

public:
	bool Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress, const std::string & AppKey, const std::string & AppSecret);
	void Clean();
	void Tick();

public:
	enum struct eState {
		Unspecified = 0,
		SetupReady  = 1,
		Connecting  = 2,
		Connected   = 3,
		Challenging = 4,
		Ready       = 5,
		Closing     = 6,
	};

	void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	void   OnFlush(xTcpConnection * TcpConnectionPtr) override;
	size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;
	bool   PostData(const void * Data, size_t DataSize);

protected:
	eState              State = eState::Unspecified;
	xel::xIoContext *   ICP;
	xel::xTcpConnection Connection;
	xel::xNetAddress    TargetAddress;
	uint64_t            LastConnectStartTimestamp;

	uint64_t MinConnectionTimeoutMS = 5'000;
	uint64_t MinReconnectTimeoutMS  = 3'000;

	// challenge data:
	std::string AppKey;
	std::string AppSecret;

	// self-auditing
	uint64_t DroppedPackets;
};
