#pragma once
#include "../base.hpp"

struct xCC_DeviceChallenge : xBinaryMessage {
    void SerializeMembers() override {
        W(AppVersion, Timestamp, Sign);
    };

    void DeserializeMembers() override {
        R(AppVersion, Timestamp, Sign);
    };

    uint32_t    AppVersion = 0;
    uint64_t    Timestamp  = 0;
    std::string Sign;
};

struct xCC_DeviceChallengeResp : xBinaryMessage {
    void SerializeMembers() override {
        W(CtrlAddress, DataAddress, CheckKey, UseOldVersion);
    };

    void DeserializeMembers() override {
        R(CtrlAddress, DataAddress, CheckKey, UseOldVersion);
    };

    xNetAddress CtrlAddress;
    xNetAddress DataAddress;
    std::string CheckKey;
    bool        UseOldVersion = false;
    //
};
