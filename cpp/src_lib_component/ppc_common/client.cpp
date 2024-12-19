#include "./client.hpp"

#include <algorithm>

static constexpr const uint64_t DefaultReconnectIntervalMS = 15'000;
// debug:
static constexpr const uint64_t DefaultKeepAliveRequestIntervalMS = 5'000;
// dist:
// static constexpr const uint64_t DefaultKeepAliveRequestIntervalMS = 75'000;
static constexpr const uint64_t DefaultKeepAliveResponseTimeoutMS = 90'000;

bool xPP_Client::Init(xIoContext * ICP) {
	RuntimeAssert(ICP);
	this->ICP = ICP;
	RuntimeAssert(DebugIdManager.Init());
	return true;
}

void xPP_Client::Clean() {
	auto ActiveClientNodeList = decltype(ReadyClientNodeList)();
	ActiveClientNodeList.GrabListTail(ConnectingNodeList);
	ActiveClientNodeList.GrabListTail(ReadyClientNodeList);
	while (auto PC = static_cast<xPP_Connection *>(ReadyClientNodeList.PopHead())) {
		PC->Clean();
		FreeConnection(PC);
	}
	while (auto PC = static_cast<xPP_Connection *>(ReconnectClientNodeList.PopHead())) {
		FreeConnection(PC);
	}
}

xPP_Connection * xPP_Client::AllocConnection() {
	auto PC = new xPP_Connection();
	// NOTICE: this is for debug use, invalid ConnectionId is allowed
	auto CID              = DebugIdManager.Acquire();
	PC->DebugConnectionId = CID;
	X_DEBUG_PRINTF("Id=%" PRIu32 "", CID);
	return PC;
}

void xPP_Client::FreeConnection(xPP_Connection * PC) {
	auto CID = Steal(PC->DebugConnectionId);
	if (CID) {
		DebugIdManager.Release(CID);
	}
	X_DEBUG_PRINTF("Id=%" PRIu32 "", CID);
	delete PC;
}

void xPP_Client::UpdateServerAddresses(const std::vector<xNetAddress> & ServerAddressesInput) {
	auto ServerAddresses = ServerAddressesInput;
	std::ranges::sort(ServerAddresses);
	auto [dup_first, dup_last] = std::ranges::unique(ServerAddresses);
	ServerAddresses.erase(dup_first, dup_last);

	this->ServerAddresses = ServerAddresses;
	auto SurvivedNodeList = decltype(ReadyClientNodeList)();
	while (auto PC = static_cast<xPP_Connection *>(ReadyClientNodeList.PopHead())) {
		bool Found = false;
		for (auto Iter = ServerAddresses.begin(); Iter != ServerAddresses.end(); ++Iter) {
			if (PC->ServerAddress == *Iter) {
				ServerAddresses.erase(Iter);
				Found = true;
				break;
			}
		}
		if (Found) {
			SurvivedNodeList.AddTail(*PC);
			continue;
		} else {
			PC->Clean();
			FreeConnection(PC);
		}
	}
	ReadyClientNodeList.GrabListHead(SurvivedNodeList);

	// rebuild new connection list:
	while (auto PC = static_cast<xPP_Connection *>(ConnectingNodeList.PopHead())) {
		PC->Clean();
		FreeConnection(PC);
	}
	while (auto PC = static_cast<xPP_Connection *>(ReconnectClientNodeList.PopHead())) {
		FreeConnection(PC);
	}
	for (auto & SA : ServerAddresses) {
		auto NPC           = AllocConnection();
		NPC->ServerAddress = SA;
		ReconnectClientNodeList.AddTail(*NPC);
	}
}

void xPP_Client::Tick() {
	NowMS = GetTimestampMS();
	do {
		// destroy closing connections
		while (auto Real = static_cast<xPP_Connection *>(ClosingClientNodeList.PopHead())) {
			xList<xPP_ConnectionNode>::Remove(*Real);
			xList<xPP_ConnectionKeepAliveNode>::Remove(*Real);
			Real->Clean();
			ReconnectClientNodeList.GrabTail(*Real);
		}

		if (NowMS - LastReconnectTimestampMS < DefaultReconnectIntervalMS) {
			break;
		}
		// clean timeout connections:
		while (auto Real = static_cast<xPP_Connection *>(ConnectingNodeList.PopHead())) {
			X_DEBUG_PRINTF("");
			Real->Clean();
			ConnectingNodeList.GrabTail(*Real);
		}
		// reconnect
		while (auto Real = static_cast<xPP_Connection *>(ReconnectClientNodeList.PopHead())) {
			RuntimeAssert(Real->Init(ICP, Real->ServerAddress, this));
			ConnectingNodeList.GrabTail(*Real);
		}
		LastReconnectTimestampMS = NowMS;
	} while (false);

	do {  // keepalive
		auto KeepAliveTimepoint = NowMS - DefaultKeepAliveRequestIntervalMS;
		while (auto RPC = ReadyClientKeepAliveNodeList.PopHead([=](const xPP_ConnectionKeepAliveNode & N) {
			return N.LastKeepAliveTimestampMS <= KeepAliveTimepoint;
		})) {
			auto Real = static_cast<xPP_Connection *>(RPC);
			X_DEBUG_PRINTF("PostKeepAliveRequest, Id=%" PRIu32 "", Real->DebugConnectionId);
			Real->LastKeepAliveTimestampMS = NowMS;
			Real->PostRequestKeepAlive();
			ReadyClientKeepAliveCheckingNodeList.GrabTail(*Real);
		}
	} while (false);

	do {  // keepalive timeout:
		auto KeepAliveTimeoutTimepoint = NowMS - DefaultKeepAliveResponseTimeoutMS;
		while (auto RPC = ReadyClientKeepAliveCheckingNodeList.PopHead([=](const xPP_ConnectionKeepAliveNode & N) {
			return N.LastKeepAliveTimestampMS <= KeepAliveTimeoutTimepoint;
		})) {
			auto Real = static_cast<xPP_Connection *>(RPC);
			Real->Clean();
			ReconnectClientNodeList.GrabTail(*Real);
		}
	} while (false);
}

void xPP_Client::OnConnected(xTcpConnection * TcpConnectionPtr) {
	X_DEBUG_PRINTF("");
	auto Real = static_cast<xPP_Connection *>(TcpConnectionPtr);
	ReadyClientNodeList.GrabTail(*Real);
	Real->LastKeepAliveTimestampMS = 0;
	ReadyClientKeepAliveNodeList.GrabHead(*Real);
}

void xPP_Client::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	X_DEBUG_PRINTF("");
	auto Real = static_cast<xPP_Connection *>(TcpConnectionPtr);
	ClosingClientNodeList.GrabTail(*Real);
}

size_t xPP_Client::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
	auto   Real       = static_cast<xPP_Connection *>(TcpConnectionPtr);
	size_t RemainSize = DataSize;
	while (RemainSize >= PacketHeaderSize) {
		auto Header = xPacketHeader::Parse(DataPtr);
		if (!Header) { /* header error */
			return InvalidDataSize;
		}
		auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
		if (RemainSize < PacketSize) {        // wait for data
			break;
		}
		if (Header.IsKeepAlive()) {
			X_DEBUG_PRINTF("KeepAlive response");
			Real->LastKeepAliveTimestampMS = NowMS;
			ReadyClientKeepAliveNodeList.GrabTail(*Real);
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnPacket(Header, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

bool xPP_Client::OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF("CommandId: %" PRIu32 ", RequestId:%" PRIx64 ": \n%s", Header.CommandId, Header.RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
	return true;
}

xPP_Connection * xPP_Client::PickConnection() {
	auto PC = static_cast<xPP_Connection *>(ReadyClientNodeList.PopHead());
	if (PC) {
		ReadyClientNodeList.AddTail(*PC);
	}
	return PC;
}
