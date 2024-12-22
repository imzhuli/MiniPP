#pragma once
#include <pp_common/base.hpp>

class xInitDataStream : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(CtrlId, DataKey);
    }
    void DeserializeMembers() override {
        R(CtrlId, DataKey);
    }

public:
    uint64_t CtrlId;
    string   DataKey;
};

class xInitDataStreamResp : public xBinaryMessage {
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
