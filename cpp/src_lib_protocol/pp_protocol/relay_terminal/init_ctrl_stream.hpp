#pragma once
#include <pp_common/base.hpp>

class xInitCtrlStream : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(CheckKey);
	}
	void DeserializeMembers() override {
		R(CheckKey);
	}

public:
	string CheckKey;
};

class xInitCtrlStreamResp : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(DeviceId, CtrlId, DeviceKey);
	}
	void DeserializeMembers() override {
		R(DeviceId, CtrlId, DeviceKey);
	}

public:
	uint64_t DeviceId;
	uint64_t CtrlId;
	string   DeviceKey;
};
