#pragma once
#include <pp_common/base.hpp>

class xTR_PostUdpData : public xBinaryMessage {
public:
	void SerializeMembers() override {
		assert(PayloadView.data() && PayloadView.size());
		W(TerminalSideChannelId, RelaySideChannelId, PayloadView);
	}
	void DeserializeMembers() override {
		R(TerminalSideChannelId, RelaySideChannelId, PayloadView);
	}

public:
	uint64_t         TerminalSideChannelId;
	uint64_t         RelaySideChannelId;
	std::string_view PayloadView;

	static constexpr const size32_t MAX_PAYLOAD_SIZE = 4096;
	static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 32);
};
