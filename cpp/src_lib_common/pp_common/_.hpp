#pragma once
#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/core_time.hpp>
#include <core/executable.hpp>
#include <core/indexed_storage.hpp>
#include <core/list.hpp>
#include <core/string.hpp>
#include <network/net_address.hpp>
#include <network/tcp_connection.hpp>
#include <network/tcp_server.hpp>
#include <network/udp_channel.hpp>
#include <object/object.hpp>
#include <server_arch/message.hpp>

//
#include <cinttypes>

// type-defs
using namespace xel::common;

using xel::InvalidDataSize;
using xel::MaxPacketPayloadSize;
using xel::MaxPacketSize;
using xel::PacketHeaderSize;
using xel::xAbstract;
using xel::xBinaryMessage;
using xel::xCommandLine;
using xel::xIndexedStorage;
using xel::xIndexId;
using xel::xIndexIdPool;
using xel::xIoContext;
using xel::xList;
using xel::xListNode;
using xel::xNetAddress;
using xel::xObjectIdManager;
using xel::xObjectIdManagerMini;
using xel::xPacket;
using xel::xPacketCommandId;
using xel::xPacketHeader;
using xel::xPacketRequestId;
using xel::xResourceGuard;
using xel::xScopeGuard;
using xel::xSocket;
using xel::xStreamReader;
using xel::xStreamWriter;
using xel::xTcpConnection;
using xel::xTcpServer;
using xel::xTicker;
using xel::xUdpChannel;

// functions
using xel::GetTimestampMS;
using xel::HexShow;
using xel::MakeResourceCleaner;
using xel::Pure;
using xel::RuntimeAssert;
using xel::Split;
using xel::Steal;
using xel::Todo;
using xel::Unreachable;
using xel::WritePacket;

// std-lib:
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

// min_defs:
using xGroupId      = uint32_t;
using xServerId     = uint32_t;
using xAccountId    = uint64_t;
using xTerminalId   = uint64_t;
using xConnectionId = uint64_t;
using xCountryId    = uint32_t;
using xStateId      = uint32_t;
using xProvinceId   = xStateId;
using xCityId       = uint32_t;

constexpr const uint64_t DeaultRequestTimeoutMS = 1'500;
