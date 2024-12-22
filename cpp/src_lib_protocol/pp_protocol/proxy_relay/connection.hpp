#pragma once
#include <pp_common/base.hpp>

class xPR_CreateConnection : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(RelaySideDeviceId, ProxySideConnectionId, TargetAddress);
    }
    void DeserializeMembers() override {
        R(RelaySideDeviceId, ProxySideConnectionId, TargetAddress);
    }

public:
    uint64_t    RelaySideDeviceId;
    uint64_t    ProxySideConnectionId;
    xNetAddress TargetAddress;
};

class xPR_DestroyConnection : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(ProxySideConnectionId, RelaySideConnectionId);
    }
    void DeserializeMembers() override {
        R(ProxySideConnectionId, RelaySideConnectionId);
    }

public:
    uint32_t ProxySideConnectionId;
    uint64_t RelaySideConnectionId;
};

class xPR_ConnectionStateNotify : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(ProxySideConnectionId, RelaySideConnectionId);
        W(TotalUploadedBytes, TotalDumpedBytes);
    }
    void DeserializeMembers() override {
        R(ProxySideConnectionId, RelaySideConnectionId);
        R(TotalUploadedBytes, TotalDumpedBytes);
    }

public:
    uint32_t ProxySideConnectionId;
    uint64_t RelaySideConnectionId;
    uint64_t TotalUploadedBytes = 0;
    uint64_t TotalDumpedBytes   = 0;
};