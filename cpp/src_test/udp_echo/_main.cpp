#include <core/core_min.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/udp_channel.hpp>

using namespace xel;
using namespace std;

struct xUdpEchoService : xUdpChannel::iListener {

    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override {
        cout << HexShow(DataPtr, DataSize) << endl;
        ChannelPtr->PostData(DataPtr, DataSize, RemoteAddress);
        ChannelPtr->PostData(DataPtr, DataSize, RemoteAddress);
        ChannelPtr->PostData(DataPtr, DataSize, RemoteAddress);
    }

    //
};

int main(int, char **) {

    auto IC             = xIoContext();
    auto ICG            = xResourceGuard(IC);
    auto BindAddress    = xNetAddress::Parse("0.0.0.0:7777");
    auto UdpChannel     = xUdpChannel();
    auto UdpEchoService = xUdpEchoService();
    auto UCG            = xResourceGuard(UdpChannel, &IC, BindAddress, &UdpEchoService);

    while (true) {
        IC.LoopOnce();
    }

    return 0;
}