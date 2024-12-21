#include "./connection.hpp"

/***************
 * Connection
 */
bool xRD_ConnectionBase::PostPacket(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    ubyte Buffer[MaxPacketSize];
    auto  PSize = WritePacket(CmdId, RequestId, Buffer, Message);
    PostData(Buffer, PSize);
    return true;
}
