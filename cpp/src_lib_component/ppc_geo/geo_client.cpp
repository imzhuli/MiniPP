#include "./geo_client.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/geo_info.hpp>

bool xPP_GeoClient::OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {

	return false;
}

void xPP_GeoClient::Tick() {
	xPP_Client::Tick();

	auto KillTimepoint = NowMS - DeaultRequestTimeoutMS;
	while (auto Real = RequestTimeoutList.PopHead([=](const xRequestContext & Ctx) { return Ctx.TimestampMS <= KillTimepoint; })) {
		RequestManager.Release(Real->LocalRequestId);
	}

	//
}

bool xPP_GeoClient::MakeRequest(const xNetAddress & Address) {
	if (Address.Port) {
		X_PERROR("Address Port should be zero");
		return false;
	}

	auto Id = RequestManager.Acquire();
	if (!Id) {
		return false;
	}
	auto & Context         = RequestManager[Id];
	Context.LocalRequestId = Id;
	Context.TimestampMS    = NowMS;
	RequestTimeoutList.AddTail(Context);

	// get connection:
	auto PC = PickConnection();
	if (!PC) {
		RequestManager.Release(Id);
		return false;
	}

	//
	auto Request = xGeoInfoReq();
	Request.IP   = Address;
	ubyte Buffer[MaxPacketSize];
	auto  RSize = WritePacket(Cmd_GeoQuery, Context.LocalRequestId, Buffer, Request);
	PC->PostData(Buffer, RSize);
	return true;
}
