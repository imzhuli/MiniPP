#pragma once
#include <pp_common/base.hpp>

class xTR_CreateConnection : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(RelaySideConnectionId, TargetAddress);
	}
	void DeserializeMembers() override {
		R(RelaySideConnectionId, TargetAddress);
	}

public:
	uint64_t    RelaySideConnectionId;
	xNetAddress TargetAddress;
};

class xTR_DestroyConnection : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(TerminalSideConnectionId, RelaySideConnectionId);
	}
	void DeserializeMembers() override {
		R(TerminalSideConnectionId, RelaySideConnectionId);
	}

public:
	uint32_t TerminalSideConnectionId;
	uint64_t RelaySideConnectionId;
};

class xTR_ConnectionStateNotify : public xBinaryMessage {
public:
	static constexpr int STATE_UNSEPC          = 0;
	static constexpr int STATE_ESTABLISHED     = 1;
	static constexpr int STATE_UPDATE_TRANSFER = 2;
	static constexpr int STATE_CLOSED          = 3;

	static const char * GetStateName(int State) {
		switch (State) {
			case STATE_UNSEPC:
				return "STATE_UNSEPC";
			case STATE_ESTABLISHED:
				return "STATE_ESTABLISHED";
			case STATE_UPDATE_TRANSFER:
				return "STATE_UPDATE_TRANSFER";
			case STATE_CLOSED:
				return "STATE_CLOSED";
			default:
				break;
		}
		return "INVALID_CONNECTION_STATE";
	}

public:
	void SerializeMembers() override {
		W(TerminalSideConnectionId, RelaySideConnectionId);
		W(NewState, TotalReadBytes, TotalWrittenBytes);
	}
	void DeserializeMembers() override {
		R(TerminalSideConnectionId, RelaySideConnectionId);
		R(NewState, TotalReadBytes, TotalWrittenBytes);
	}

public:
	uint32_t TerminalSideConnectionId;
	uint64_t RelaySideConnectionId;
	uint32_t NewState          = 0;
	uint64_t TotalReadBytes    = 0;
	uint64_t TotalWrittenBytes = 0;
};
