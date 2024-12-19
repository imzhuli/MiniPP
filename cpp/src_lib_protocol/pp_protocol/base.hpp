#pragma once
#include <pp_common/base.hpp>
#include <pp_common/error.hpp>

class xGroupedMessage : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(GroupId);
	}
	void DeserializeMembers() override {
		R(GroupId);
	}

public:
	xGroupId GroupId = 0;
};
