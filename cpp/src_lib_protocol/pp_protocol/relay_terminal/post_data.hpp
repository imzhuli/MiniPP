#pragma once
#include <pp_common/base.hpp>

class xTR_PostData : public xBinaryMessage {
public:
	void SerializeMembers() override {
		assert(PayloadView.data() && PayloadView.size());
		W(TerminalSideConnectionId, RelaySideConnectionId, PayloadView);
	}
	void DeserializeMembers() override {
		R(TerminalSideConnectionId, RelaySideConnectionId, PayloadView);
	}

public:
	uint64_t         TerminalSideConnectionId;
	uint64_t         RelaySideConnectionId;
	std::string_view PayloadView;

	static constexpr const size32_t MAX_PAYLOAD_SIZE = 4096;
	static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 32);
};
