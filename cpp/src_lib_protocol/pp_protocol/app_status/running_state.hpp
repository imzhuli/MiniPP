#pragma once
#include "../base.hpp"

struct xQueryRunningState : xBinaryMessage {

	virtual void SerializeMembers() override {
		W(ChallengeTimestamp, ChallengeString);
	}

	virtual void DeserializeMembers() override {
		R(ChallengeTimestamp, ChallengeString);
	}

	uint64_t    ChallengeTimestamp;
	std::string ChallengeString;

	//
};

struct xQueryRunningStateResp : xBinaryMessage {

	static constexpr const uint16_t RUNNING_OK           = 0x00;
	static constexpr const uint16_t RUNNING_HALTED       = 0x01;
	static constexpr const uint16_t RUNNING_HEAVY_LOADED = 0x02;

	virtual void SerializeMembers() override {
		W(ChallengeTimestamp, ChallengeString);
		W(RunningState);
		W(CpuUsage);
	}

	virtual void DeserializeMembers() override {
		R(ChallengeTimestamp, ChallengeString);
		R(RunningState);
		R(CpuUsage);
	}

	uint64_t    ChallengeTimestamp;
	std::string ChallengeString;
	uint16_t    RunningState = RUNNING_OK;
	uint16_t    CpuUsage     = 0;

	//
};
