#include "../common/backend_client.hpp"

#include <pp_common/base.hpp>
#include <pp_protocol/backend/auth_by_user_pass.hpp>
#include <pp_protocol/backend/backend_challenge.hpp>
#include <pp_protocol/command.hpp>
#include <server_arch/client.hpp>

static auto IC           = xIoContext();
static auto ICG          = xResourceGuard(IC);
static auto TestAddress  = xNetAddress::Parse("45.202.204.29:20005");
static auto TestAddress2 = xNetAddress::Parse("45.202.204.27:20005");

using namespace xel;
using namespace std;

class xAuthConnection : public xBackendClient {
public:
	bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress) {
		if (!xBackendClient::Init(IoContextPtr, TargetAddress)) {
			return false;
		}
		Enabled = false;
		return true;
	}

	bool OnBusinessPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override {
		switch (Header.CommandId) {
			case Cmd_AuthByUserPassResp: {
				// X_DEBUG_PRINTF("data:\n%s", HexShow(PayloadPtr, PayloadSize).c_str());
				auto crr = xBackendAuthByUserPassResp();
				if (!crr.Deserialize(PayloadPtr, PayloadSize)) {
					++AuditAuthRespError;
					// X_DEBUG_PRINTF("failed to parse response");
					// X_DEBUG_PRINTF("AuditId: %u", (unsigned)crr.AuditId);
					// X_DEBUG_PRINTF("CountryCode: %s", crr.CountryCode.c_str());
					// X_DEBUG_PRINTF("StateId: %s", crr.StateId.c_str());
					// X_DEBUG_PRINTF("CityId: %u", (unsigned)crr.CityId);
					// X_DEBUG_PRINTF("Duration: %u", (unsigned)crr.Duration);
					// X_DEBUG_PRINTF("Random: %u", (unsigned)crr.Random);
					// X_DEBUG_PRINTF("Redirect: %s", crr.Redirect.c_str());
				} else {
					++AuditAuthRespSuccess;
					// X_DEBUG_PRINTF("AuditId: %u", (unsigned)crr.AuditId);
					// X_DEBUG_PRINTF("CountryCode: %s", crr.CountryCode.c_str());
					// X_DEBUG_PRINTF("StateId: %s", crr.StateId.c_str());
					// X_DEBUG_PRINTF("CityId: %u", (unsigned)crr.CityId);
					// X_DEBUG_PRINTF("Duration: %u", (unsigned)crr.Duration);
					// X_DEBUG_PRINTF("Random: %u", (unsigned)crr.Random);
					// X_DEBUG_PRINTF("Redirect: %s", crr.Redirect.c_str());
				}
				if (LastResponseId < Header.RequestId) {
					LastResponseId = Header.RequestId;
				}

				break;
			}
			default: {
				X_PERROR("invalid command : %x", (unsigned)Header.CommandId);
				break;
			}
		}
		return true;
	}

	void DoAuth(const std::string_view UserView, const std::string_view PassView) {
		auto Auth     = xBackendAuthByUserPass();
		Auth.UserPass = "B_1_US_1583_19158_30_tWkmYP8g:1234567";
		ubyte Buffer[MaxPacketSize];
		auto  RSize = WritePacket(Cmd_AuthByUserPass, RequestId++, Buffer, sizeof(Buffer), Auth);
		PostData(Buffer, RSize);

		++AuditRequested;
	}

private:
	bool     Enabled   = false;
	uint64_t RequestId = 0;

public:
	size_t   AuditRequested       = 0;
	size_t   AuditAuthRespError   = 0;
	size_t   AuditAuthRespSuccess = 0;
	uint64_t LastResponseId       = 0;
};

int main(int, char **) {
	RuntimeAssert(ICG);
	auto AC1  = xAuthConnection();
	auto AC2  = xAuthConnection();
	auto AC3  = xAuthConnection();
	auto AC4  = xAuthConnection();
	auto AC5  = xAuthConnection();
	auto ACG1 = xResourceGuard(AC1, &IC, TestAddress);
	auto ACG2 = xResourceGuard(AC2, &IC, TestAddress);
	auto ACG3 = xResourceGuard(AC3, &IC, TestAddress);
	auto ACG4 = xResourceGuard(AC4, &IC, TestAddress2);
	auto ACG5 = xResourceGuard(AC5, &IC, TestAddress2);
	RuntimeAssert(ACG1);
	RuntimeAssert(ACG2);
	RuntimeAssert(ACG3);
	RuntimeAssert(ACG4);
	RuntimeAssert(ACG5);

	auto T     = xTimer();
	auto Round = size_t();
	while (true) {
		IC.LoopOnce();
		for (int I = 0; I < 8; ++I) {
			AC1.DoAuth("", "");
			AC2.DoAuth("", "");
			AC3.DoAuth("", "");
			AC4.DoAuth("", "");
			AC5.DoAuth("", "");
		}
		AC1.Tick(GetTimestampMS());
		AC2.Tick(GetTimestampMS());
		AC3.Tick(GetTimestampMS());
		AC4.Tick(GetTimestampMS());
		AC5.Tick(GetTimestampMS());

		if (T.TestAndTag(1s)) {
			cout << "================" << ++Round << endl;
			cout << ": " << Steal(AC1.AuditRequested) << ", " << Steal(AC1.AuditAuthRespSuccess) << ", " << Steal(AC1.AuditAuthRespError)
				 << " L: " << AC1.LastResponseId << endl;
			cout << ": " << Steal(AC2.AuditRequested) << ", " << Steal(AC2.AuditAuthRespSuccess) << ", " << Steal(AC2.AuditAuthRespError)
				 << " L: " << AC2.LastResponseId << endl;
			cout << ": " << Steal(AC3.AuditRequested) << ", " << Steal(AC3.AuditAuthRespSuccess) << ", " << Steal(AC3.AuditAuthRespError)
				 << " L: " << AC3.LastResponseId << endl;
			cout << ": " << Steal(AC4.AuditRequested) << ", " << Steal(AC4.AuditAuthRespSuccess) << ", " << Steal(AC4.AuditAuthRespError)
				 << " L: " << AC4.LastResponseId << endl;
			cout << ": " << Steal(AC5.AuditRequested) << ", " << Steal(AC5.AuditAuthRespSuccess) << ", " << Steal(AC5.AuditAuthRespError)
				 << " L: " << AC5.LastResponseId << endl;
		}
	}
	return 0;
}
