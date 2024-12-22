#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>
//
#include <iostream>

//
using namespace std;
using namespace xel;

auto RelayAddress          = xNetAddress::Parse("127.0.0.1:17002");
auto TargetAddress         = xNetAddress::Parse("183.2.172.42:80");                       // ipv4
auto TargetAddress6        = xNetAddress::Parse("[240e:ff:e020:9ae:0:ff:b014:8e8b]:80");  // ipv6
auto Payload               = std::string("GET / HTTP/1.1\r\n\r\n");
auto TargetDeviceId        = uint64_t(0);
auto ProxySideConnectionId = uint64_t(12345);

auto RF  = xRunState();
auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct xPA_Listener : xTcpConnection::iListener {
    xPA_Listener(const xNetAddress & RelayServerAddress, const xNetAddress & TargetAddress, const std::string & Request) {
        cout << "relay server address: " << RelayServerAddress.ToString() << endl;
        cout << "target address: " << TargetAddress.ToString() << endl;
        cout << "request: \n------ BEGIN ------ \n" << Request << endl << "------ END ------" << endl;
        RuntimeAssert(Connection.Init(&IC, RelayServerAddress, this));
        this->TargetAddress = TargetAddress;
        this->Request       = Request;
    }
    ~xPA_Listener() {
        Connection.Clean();
    }

    void OnConnected(xTcpConnection * TcpConnectionPtr) override {
        cout << "connected" << endl;
        auto C          = xPR_Challenge();
        C.Timestamp     = GetTimestampMS();
        C.ChallengeKey  = "Hello";
        C.ChallengeHash = "World";
        ubyte Buffer[MaxPacketSize];
        auto  RSize = WritePacket(Cmd_PA_RL_Challenge, 0, Buffer, C);
        TcpConnectionPtr->PostData(Buffer, RSize);

        Connected = true;
    }

    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override {
        cout << "peer close" << endl;
        Connected = false;
        RF.Stop();
    }

    void Tick() {
        if (!Connected) {
            return;
        }
        if (!RequireConnection) {
            auto R                  = xPR_CreateConnection();
            R.RelaySideDeviceId     = TargetDeviceId;
            R.TargetAddress         = TargetAddress6;
            R.ProxySideConnectionId = ProxySideConnectionId;

            ubyte Buffer[MaxPacketSize];
            auto  RSize = WritePacket(Cmd_PA_RL_CreateConnection, 0, Buffer, R);
            Connection.PostData(Buffer, RSize);
            RequireConnection = true;
            return;
        }
    }

    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override {
        cout << "Data:\n" << HexShow(DataPtr, DataSize) << endl;
        return DataSize;
    }

    xTcpConnection Connection;
    xNetAddress    TargetAddress;
    std::string    Request;

    bool Connected         = false;
    bool RequireConnection = false;
};

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'd', nullptr, "device_id", true },
            { 'c', nullptr, "config", true },
        }
    );

    auto Did = CL["device_id"];
    RuntimeAssert(Did(), "require device id");
    cout << "input device id:" << *Did << endl;
    TargetDeviceId = atoll(Did->c_str());
    cout << "DeviceId:" << TargetDeviceId << endl;

    cout << RelayAddress.ToString() << endl;
    cout << TargetAddress.ToString() << endl;
    cout << TargetAddress6.ToString() << endl;
    cout << Payload << endl;

    RF.Start();
    auto PAL = xPA_Listener(RelayAddress, TargetAddress, Payload);
    while (RF) {
        IC.LoopOnce();
        PAL.Tick();
    }
    RF.Finish();

    return 0;
}
