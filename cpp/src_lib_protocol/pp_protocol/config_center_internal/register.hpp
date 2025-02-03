#pragma once
#include <pp_common/base.hpp>
#include <pp_common/error.hpp>

class xCC_Register : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(ChallengeString);
    }
    void DeserializeMembers() override {
        R(ChallengeString);
    }

public:
    std::string ChallengeString;
};

class xCC_RegisterResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(Accepted);
    }
    void DeserializeMembers() override {
        R(Accepted);
    }

public:
    bool Accepted;
};
