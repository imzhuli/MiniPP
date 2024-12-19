#pragma once
#include <pp_common/base.hpp>

class xBackendAuthByUserPass : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(UserPass);
	}
	void DeserializeMembers() override {
		R(UserPass);
	}

public:
	std::string UserPass;
};

class xBackendAuthByUserPassResp : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(ErrorCode);
		if (!ErrorCode) {
			R(AuditId, CountryCode, StateId, CityId, Duration, Random, Redirect);
		}
	}
	void DeserializeMembers() override {
		R(ErrorCode);
		if (!ErrorCode) {
			R(AuditId, CountryCode, StateId, CityId, Duration, Random, Redirect);
		}
	}

public:
	uint32_t    ErrorCode;
	uint32_t    AuditId;
	std::string CountryCode;
	std::string StateId;
	uint32_t    CityId;
	uint32_t    Duration;
	uint32_t    Random;
	std::string Redirect;
};
