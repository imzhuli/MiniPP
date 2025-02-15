#pragma once
#include "../base.hpp"

struct xCCR_GetServerList : xBinaryMessage {

    void SerializeMembers() override;
    void DeserializeMembers() override;

    uint64_t TimestampMS;
};

struct xCCR_GetServerListResp : xBinaryMessage {
    void SerializeMembers() override {
        auto Count = (uint32_t)Addresses.size();
        W(Count);
        for (auto & A : Addresses) {
            W(A);
        }
    };

    void DeserializeMembers() override {
        uint32_t Count = 0;
        R(Count);
        Addresses.resize(Count);
        for (auto & A : Addresses) {
            R(A);
        }
    }

    std::vector<xNetAddress> Addresses;
};
