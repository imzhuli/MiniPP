#pragma once
#include "./command.hpp"

struct xProxyClientAuth : xBinaryMessage {
    void SerializeMembers() {
        W(AddressString, AccountName, Password);
    }
    void DeserializeMembers() {
        R(AddressString, AccountName, Password);
    }
    std::string AddressString;
    std::string AccountName;
    std::string Password;
};

struct xProxyClientAuthResp : xBinaryMessage {
    void SerializeMembers() {
        W(AuditKey, CacheTimeout);
        W(TerminalControllerAddress, TerminalId);
        W(EnableUdp);
    }
    void DeserializeMembers() {
        R(AuditKey, CacheTimeout);
        R(TerminalControllerAddress, TerminalId);
        R(EnableUdp);
    }
    uint64_t    AuditKey;                   //
    uint64_t    CacheTimeout;               //
    xNetAddress TerminalControllerAddress;  // relay server, or terminal service address
    uint64_t    TerminalId;                 // index in relay server
    bool        EnableUdp;
};
