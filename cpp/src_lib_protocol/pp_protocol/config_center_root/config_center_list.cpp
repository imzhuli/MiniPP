#include "./config_center_list.hpp"

#include <crypto/md5.hpp>

static std::string Check(uint64_t TimestampMS) {
    auto V = std::to_string(TimestampMS) + "_VZXC$%^&*";
    auto H = Md5(V.data(), V.size());
    return std::string((const char *)H.Data(), H.Size());
}

void xCCR_GetServerList::SerializeMembers() {
    W(TimestampMS, Check(TimestampMS));
}

void xCCR_GetServerList::DeserializeMembers() {
    std::string H;
    R(TimestampMS, H);
    if (H != Check(TimestampMS)) {
        GetReader()->SetError();
        return;
    }
}
