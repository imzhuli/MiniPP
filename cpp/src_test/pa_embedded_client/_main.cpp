#include <pp_common/_.hpp>

//
#include <iostream>

//
using namespace std;
using namespace xel;

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
        TcpConnectionPtr->PostKeepAlive();
    }

    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override {
        cout << "peer close" << endl;
        RF.Stop();
    }

    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override {
        cout << "Data:\n" << HexShow(DataPtr, DataSize) << endl;
        return DataSize;
    }

    xTcpConnection Connection;
    xNetAddress    TargetAddress;
    std::string    Request;
};

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'r', nullptr, "relay_address", true },
            { 't', nullptr, "target_address", true },
            { 'd', nullptr, "request_data", true },
        }
    );

    auto RA      = CL["relay_address"];
    auto TA      = CL["target_address"];
    auto Payload = CL["request_data"];
    RuntimeAssert(RA(), "require ra");
    RuntimeAssert(TA(), "require_ta");
    if (!Payload()) {
        Payload = "GET / HTTP/1.1\r\n\r\n";
    }

    RF.Start();
    auto PAL = xPA_Listener(xNetAddress::Parse(*RA), xNetAddress::Parse(*TA), *Payload);
    while (RF) {
        IC.LoopOnce();
    }
    RF.Finish();

    return 0;
}
