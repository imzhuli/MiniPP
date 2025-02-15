#pragma once
#include "../base.hpp"

class xGetBackendServerList : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(Version, StartId);
    };

    void DeserializeMembers() override {
        R(Version, StartId);
    };

public:
    uint32_t Version = 0;
    uint32_t StartId = 0;
};

class xGetBackendServerListResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        auto Count = (uint32_t)AddressList.size();
        W(Version, NextId, Count);
        for (auto & A : AddressList) {
            W(A);
        }
    };

    void DeserializeMembers() override {
        AddressList.clear();
        uint32_t Count;
        R(Version, NextId, Count);
        for (uint32_t I = 0; I < Count; ++I) {
            auto Addr = xNetAddress();
            R(Addr);
            if (!Addr) {
                GetReader()->SetError();
                return;
            }
            AddressList.push_back(Addr);
        }
    };

public:
    uint32_t                 Version = 0;
    uint32_t                 NextId  = 0;
    std::vector<xNetAddress> AddressList;
};
