#include "./report_connection.hpp"

#include <pp_protocol/command.hpp>

bool xAuditReportConnection::Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress, const std::string & AppKey, const std::string & AppSecret) {
	State                           = eState::Unspecified;
	this->ICP                       = ICP;
	this->TargetAddress             = TargetAddress;
	this->State                     = eState::SetupReady;
	this->LastConnectStartTimestamp = 0;

	this->AppKey    = AppKey;
	this->AppSecret = AppSecret;

	this->DroppedPackets = 0;
	return true;
}

void xAuditReportConnection::Clean() {
	if (State >= eState::Connecting && State <= eState::Closing) {
		Connection.Clean();
	}
	State = eState::Unspecified;
}

void xAuditReportConnection::Tick() {
	assert(State != eState::Unspecified);
	if (State == eState::Connecting || State == eState::Connected || State == eState::Challenging) {
		// check connection timeout
		auto NowMS = xel::GetTimestampMS();
		if ((NowMS - LastConnectStartTimestamp) >= MinConnectionTimeoutMS) {
			State = eState::Closing;
		}
		// pass through
	}
	if (State == eState::Closing) {
		Connection.Clean();
		State = eState::SetupReady;
		// pass through
	}
	if (State == eState::SetupReady) {
		// reconnect:
		auto NowMS = xel::GetTimestampMS();
		if (SignedDiff(NowMS, LastConnectStartTimestamp) < MakeSigned(MinReconnectTimeoutMS)) {
			return;
		}
		X_DEBUG_PRINTF("Trying to connect");
		LastConnectStartTimestamp = NowMS;
		if (!Connection.Init(ICP, TargetAddress, this)) {
			return;
		}
		State = eState::Connecting;
	}
	return;
}

void xAuditReportConnection::OnConnected(xTcpConnection * TcpConnectionPtr) {

	X_DEBUG_PRINTF("");

	State                    = eState::Connected;
	auto challenge           = xBackendChallenge();
	challenge.AppKey         = AppKey;
	challenge.TimestampMS    = xel::GetTimestampMS();
	challenge.ChallengeValue = challenge.GenerateChallengeString(AppSecret);
	ubyte Buffer[xel::MaxPacketSize];
	auto  RSize = xel::WritePacket(Cmd_BackendChallenge, 0, Buffer, sizeof(Buffer), challenge);
	Connection.PostData(Buffer, RSize);

	X_DEBUG_PRINTF("Sending:\n%s", HexShow(Buffer, RSize).c_str());
	X_DEBUG_PRINTF("Header: %s", StrToHex(Buffer, PacketHeaderSize).c_str());
	X_DEBUG_PRINTF("Body: %s", StrToHex(Buffer + PacketHeaderSize, RSize - PacketHeaderSize).c_str());
	State = eState::Challenging;
}

void xAuditReportConnection::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	X_DEBUG_PRINTF("");
}

void xAuditReportConnection::OnFlush(xTcpConnection * TcpConnectionPtr) {
	X_DEBUG_PRINTF("");
}

size_t xAuditReportConnection::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
	X_DEBUG_PRINTF("OnData: \n%s", HexShow(DataPtr, DataSize).c_str());
	return DataSize;
}

bool xAuditReportConnection::PostData(const void * Data, size_t DataSize) {
	if (State != eState::Ready) {
		return false;
	}
	Connection.PostData(Data, DataSize);
	return true;
}
