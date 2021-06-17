//------------------------------------------------------------------------------------------------
// File: CommandDecoder.cpp
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to decode packets
//------------------------------------------------------------------------------------------------
#if  defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "CommandInterface.h"
#include "CommandDecoder.h"

void CommandDecoder::DecodeCommonHeader(S_MSG_CMN_HDR* pstMsgCmnHdr)
{
    pstMsgCmnHdr->msgType = ntohs(pstMsgCmnHdr->msgType);
    pstMsgCmnHdr->mode = ntohs(pstMsgCmnHdr->mode);
    pstMsgCmnHdr->msgId = ntohl(pstMsgCmnHdr->msgId);
    pstMsgCmnHdr->dataLen = ntohl(pstMsgCmnHdr->dataLen);
}

//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------


