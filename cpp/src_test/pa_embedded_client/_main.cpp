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
auto PushPayload           = std::string("GET / HTTP/1.1\r\n\r\n");
auto TargetDeviceId        = uint64_t(0);
auto ProxySideConnectionId = uint64_t(12345);
auto RelaySideConnectionId = uint64_t(0);
auto CloseTimer            = xTimer();

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
        CloseTimer.Tag();
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
        if (!HeaderPushed) {
            Pass();
        }
        if (!ConnectionClosed && CloseTimer.TestAndTag(2s)) {
            X_DEBUG_PRINTF("try to close connection");
            auto R                  = xPR_DestroyConnection();
            R.RelaySideConnectionId = RelaySideConnectionId;
            R.ProxySideConnectionId = ProxySideConnectionId;

            ubyte Buffer[MaxPacketSize];
            auto  RSize = WritePacket(Cmd_PA_RL_DestroyConnection, 0, Buffer, R);
            Connection.PostData(Buffer, RSize);

            ConnectionClosed = true;
        }
    }

    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override {
        size_t RemainSize = DataSize;
        while (RemainSize >= PacketHeaderSize) {
            auto Header = xPacketHeader::Parse(DataPtr);
            if (!Header) { /* header error */
                return InvalidDataSize;
            }
            auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
            if (RemainSize < PacketSize) {        // wait for data
                break;
            }
            if (Header.IsKeepAlive()) {
                X_DEBUG_PRINTF("KeepAlive");
            } else {
                auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
                auto PayloadSize = Header.GetPayloadSize();
                if (!OnPacket(TcpConnectionPtr, Header, PayloadPtr, PayloadSize)) {
                    return InvalidDataSize;
                }
            }
            DataPtr    += PacketSize;
            RemainSize -= PacketSize;
        }
        return DataSize - RemainSize;
    }

    bool OnPacket(xTcpConnection * Conn, const xPacketHeader & Header, ubyte * Payload, size_t PayloadSize) {
        switch (Header.CommandId) {
            case Cmd_PA_RL_ChallengeResp:
                X_DEBUG_PRINTF("Cmd_PA_RL_ChallengeResp");
                return true;
            case Cmd_PA_RL_NotifyConnectionState:
                X_DEBUG_PRINTF("Cmd_PA_RL_NotifyConnectionState");
                return OnConnectionNotify(Conn, Header, Payload, PayloadSize);
            case Cmd_PA_RL_PostData:
                X_DEBUG_PRINTF("Cmd_PA_RL_PostData");
                return OnConnectionData(Conn, Header, Payload, PayloadSize);
            default:
                X_DEBUG_PRINTF("Unrecognized CommandId=%" PRIx32 "", Header.CommandId);
                break;
        }
        return false;
    }

    bool OnConnectionNotify(xTcpConnection * Conn, const xPacketHeader & Header, ubyte * Payload, size_t PayloadSize) {
        auto R = xPR_ConnectionStateNotify();
        if (!R.Deserialize(Payload, PayloadSize)) {
            X_DEBUG_PRINTF("invalid data");
            return false;
        }
        X_DEBUG_PRINTF("NewState=%" PRIi32 ", PSideId=%" PRIx64 ", RSideId=%" PRIx64 "", R.NewState, R.ProxySideConnectionId, R.RelaySideConnectionId);
        if (R.NewState == xPR_ConnectionStateNotify::STATE_ESTABLISHED) {
            X_DEBUG_PRINTF("Connection established");

            auto Push                  = xPR_PushData();
            Push.ProxySideConnectionId = R.ProxySideConnectionId;
            Push.RelaySideConnectionId = R.RelaySideConnectionId;
            Push.PayloadView           = Request;

            PostMessage(Cmd_PA_RL_PostData, 0, Push);

            RelaySideConnectionId = R.RelaySideConnectionId;
            HeaderPushed          = true;
            return true;
        } else if (R.NewState == xPR_ConnectionStateNotify::STATE_CLOSED) {
            X_DEBUG_PRINTF("Connection closed");
            RF.Stop();
            return true;
        } else if (R.NewState == xPR_ConnectionStateNotify::STATE_UPDATE_TRANSFER) {
            return true;
        }
        return false;
    }

    bool OnConnectionData(xTcpConnection * Conn, const xPacketHeader & Header, ubyte * Payload, size_t PayloadSize) {
        auto R = xPR_PushData();
        if (!R.Deserialize(Payload, PayloadSize)) {
            X_DEBUG_PRINTF("invalid protocol");
            return false;
        }
        X_DEBUG_PRINTF("Data from connection:\n%s", HexShow(R.PayloadView).c_str());
        return true;
    }

    void PostMessage(uint32_t CmdId, uint64_t RequestId, xBinaryMessage & M) {
        ubyte Buffer[MaxPacketSize];
        auto  RSize = WritePacket(CmdId, RequestId, Buffer, M);
        if (!RSize) {
            X_DEBUG_PRINTF("Failed to serialize message");
            return;
        }
        Connection.PostData(Buffer, RSize);
    }

    xTcpConnection Connection;
    xNetAddress    TargetAddress;
    std::string    Request;

    bool Connected         = false;
    bool RequireConnection = false;
    bool HeaderPushed      = false;
    bool ConnectionClosed  = false;
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

    RF.Start();
    auto PAL = xPA_Listener(RelayAddress, TargetAddress, PushPayload);
    while (RF) {
        IC.LoopOnce();
        PAL.Tick();
    }
    RF.Finish();

    return 0;
}
