//------------------------------------------------------------------------------------------------
// File: MessageDecoder.cpp
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to decode packets
//------------------------------------------------------------------------------------------------
#include <cstring>
#if  defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "CommandInterface.h"
#include "CommandDecoder.h"
#include "gssSocket.hpp"
#include "Logger.h"

void CommandDecoder::DecodeCommonHeader(S_MSG_CMN_HDR *pstMsgCmnHdr)
{
    pstMsgCmnHdr->msgType = ntohs(pstMsgCmnHdr->msgType);
    pstMsgCmnHdr->mode = ntohs(pstMsgCmnHdr->mode);
    pstMsgCmnHdr->msgId = ntohl(pstMsgCmnHdr->msgId);
    pstMsgCmnHdr->dataLen = ntohl(pstMsgCmnHdr->dataLen);

    LOGGER->Log("[Req %u], type :0x%x, mode : 0x%x,length %d", pstMsgCmnHdr->msgId, pstMsgCmnHdr->msgType, 
		  		pstMsgCmnHdr->mode, pstMsgCmnHdr->dataLen);
}

int CommandDecoder::ParsingBuffer(unsigned char* pRcvMsg, unsigned char** buf)
{
    int bufLen = 0;
    unsigned char* ptr = pRcvMsg;

    bufLen = ntohl(*(int*)pRcvMsg);
    ptr += sizeof(int);

    if(bufLen > 0)
    {
        *buf = new unsigned char[bufLen];
        memcpy(*buf, ptr, bufLen);
        ptr += bufLen;
    }

    return bufLen;
}
