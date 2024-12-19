#include "./backend_client.hpp"

#include <pp_common/base.hpp>
#include <pp_protocol/backend/backend_challenge.hpp>
#include <pp_protocol/command.hpp>

using namespace xel;
using namespace std;

static auto IC  = xIoContext();
static auto ICG = xResourceGuard(IC);
// static auto TestAuth      = "B_1_US_1583_19158_30_tWkmYP8g:1234567";
static auto TestAppKey    = "apitest";
static auto TestAppSecret = "123456";

bool xBackendClient::Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress) {
	RuntimeAssert(xCClient::Init(ICP, TargetAddress));
	SetMaxWriteBuffer(2'000'000);
	return true;
}

void xBackendClient::PostChallengeRequest() {

	X_DEBUG_PRINTF("");
	PrepareTimestampMS = GetTimestampMS();

	auto challenge           = xBackendChallenge();
	challenge.AppKey         = TestAppKey;
	challenge.TimestampMS    = xel::GetTimestampMS();
	challenge.ChallengeValue = challenge.GenerateChallengeString(TestAppSecret);

	ubyte Buffer[xel::MaxPacketSize];
	auto  RSize = xel::WritePacket(Cmd_BackendChallenge, 0, Buffer, sizeof(Buffer), challenge);
	PostData(Buffer, RSize);

	X_DEBUG_PRINTF("Sending:\n%s", HexShow(Buffer, RSize).c_str());
	X_DEBUG_PRINTF("Header: %s", StrToHex(Buffer, PacketHeaderSize).c_str());
	X_DEBUG_PRINTF("Body: %s", StrToHex(Buffer + PacketHeaderSize, RSize - PacketHeaderSize).c_str());
}

bool xBackendClient::OnChallengeResponse(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	if (Header.CommandId != Cmd_BackendChallengeResp) {
		return false;
	}
	auto cr = xBackendChallengeResp();
	if (!cr.Deserialize(PayloadPtr, PayloadSize) || cr.ErrorCode) {
		X_DEBUG_PRINTF("Failed to challenge backend server");
		return false;
	}
	X_DEBUG_PRINTF("backend server accepted");
	return true;
}
