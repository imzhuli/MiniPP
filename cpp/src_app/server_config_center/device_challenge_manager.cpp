#include "./device_challenge_manager.hpp"

#include "./config.hpp"
#include "./global.hpp"

#include <crypto/md5.hpp>
#include <network/udp_channel.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/config_center/terminal.hpp>

using xel::xUdpChannel;

static xUdpChannel DeviceChallengeChannel;

static const xCC_DeviceRelayServerInfoBase * GetRelayServerByDevice(const xNetAddress & RemoteAddress) {
	return &RelayForTest;
}

static struct xDeviceChallengeProcessor : xUdpChannel::iListener {
	void OnError(xUdpChannel * ChannelPtr) override {
	}
	void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override {
		if (DataSize < PacketHeaderSize) {
			X_DEBUG_PRINTF("Invalid packet size");
			return;
		}
		auto Header = xPacketHeader();
		Header.Deserialize(DataPtr);
		if (Header.PacketSize != DataSize) {
			X_DEBUG_PRINTF("Invalid packet size from header");
			return;
		}

		X_DEBUG_PRINTF("Request: CmdId=%" PRIx32 ", RequestId=%" PRIx64 ", RemoteAddress=%s", Header.CommandId, Header.RequestId, RemoteAddress.ToString().c_str());
		auto Payload     = xPacket::GetPayloadPtr(DataPtr);
		auto PayloadSize = Header.GetPayloadSize();
		Touch(PayloadSize);

		X_DEBUG_PRINTF("Request body: \n%s", HexShow(Payload, PayloadSize).c_str());
		switch (Header.CommandId) {
			case Cmd_Terminal_CC_Challenge: {
				OnTerminalChallenge(ChannelPtr, Payload, PayloadSize, RemoteAddress);
				break;
			}

			default:
				X_DEBUG_PRINTF("Unrecognized command");
				break;
		}

		return;
	}

	void OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress) {
		auto Request = xCC_DeviceChallenge();
		if (!Request.Deserialize(Payload, PayloadSize)) {
			X_DEBUG_PRINTF("Invalid requst format");
		}

		X_DEBUG_PRINTF("Version:%" PRIu32 ", Timestamp:%" PRIu64 ", Sign=%s", Request.AppVersion, Request.Timestamp, Request.Sign.c_str());
		// check sign:
		auto Source = "TLMPP1" + std::to_string(Request.Timestamp);
		auto Digest = Md5(Source.data(), Source.size());
		if (Request.Sign != StrToHex(Digest.Data(), Digest.Size())) {
			X_DEBUG_PRINTF("Invalid sign");
			return;
		}

		X_DEBUG_PRINTF("Challenge accepted");
		auto RSP            = GetRelayServerByDevice(RemoteAddress);
		auto Resp           = xCC_DeviceChallengeResp();
		Resp.ControlAddress = RSP->ControlAddress;
		Resp.DataAddress    = RSP->DataAddress;
		Resp.CheckKey       = "TLMPP-FOR-TEST";

		ubyte Buffer[MaxPacketSize];
		auto  RSize = WritePacket(Cmd_Terminal_CC_ChallengeResp, 0, Buffer, Resp);

		X_DEBUG_PRINTF("Post Response to %s\n%s", RemoteAddress.ToString().c_str(), HexShow(Buffer, RSize).c_str());
		ChannelPtr->PostData(Buffer, RSize, RemoteAddress);
	}

	//
} DeviceChallengeProcessor;

bool InitDeviceChallengeManager() {
	RuntimeAssert(GlobalIoContext());
	RuntimeAssert(DeviceChallengeChannel.Init(&GlobalIoContext, BindAddressForDevice, &DeviceChallengeProcessor));
	printf("DeviceChallengeProcessor bind address: %s\n", BindAddressForDevice.ToString().c_str());
	return true;
}

void CleanDeviceChallengeManager() {
	DeviceChallengeChannel.Clean();
}
