#pragma once
#include "./command.hpp"

struct xGeoInfoReq : xBinaryMessage {
	void SerializeMembers() override {
		W(IP);
	}
	void DeserializeMembers() override {
		R(IP);
	}
	xNetAddress IP;
};

struct xGeoInfoResp : xBinaryMessage {
	void SerializeMembers() override {
		W(IP, CountryId, CityId, CityName, Signature);
	}
	void DeserializeMembers() override {
		R(IP, CountryId, CityId, CityName, Signature);
	}

	std::string IP;
	uint32_t    CountryId;
	uint32_t    CityId;
	std::string CityName;
	std::string Signature;
};
