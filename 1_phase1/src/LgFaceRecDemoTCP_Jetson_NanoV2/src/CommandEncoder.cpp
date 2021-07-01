//------------------------------------------------------------------------------------------------
// File: MessageEncoder.cpp
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to encode packets
//------------------------------------------------------------------------------------------------

#include <cstring>
#if  defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "CommandInterface.h"
#include "CommandEncoder.h"
#include "gssSocket.hpp"

static  int init_values[2] = { cv::IMWRITE_JPEG_QUALITY,80 }; //default(95) 0-100
static  std::vector<int> param (&init_values[0], &init_values[0]+2);
static  std::vector<uchar> sendbuff;//buffer for coding

int CommandEncoder::SendStreamData(gssSocketClass *pSocket, cv::Mat Image)
{
    int msgLen = 0;
    unsigned int imagesize = 0;
    
    cv::imencode(".jpg", Image, sendbuff, param);
    imagesize = sendbuff.size(); // convert image size to network format

    msgLen = sizeof(S_MSG_CMN_HDR) + imagesize;

	unsigned char* pMsg = new unsigned char[msgLen];
	unsigned char* ptr = pMsg;
	memset(pMsg, 0x00, msgLen);

	// Header
	S_MSG_CMN_HDR* pstMsgHdr = (S_MSG_CMN_HDR*)pMsg;

	pstMsgHdr->msgType = htons(MSG_TYPE_STREAM);
	pstMsgHdr->mode = 0;
	pstMsgHdr->msgId = 0; 
	pstMsgHdr->dataLen = htonl(imagesize);
	
	ptr += sizeof(S_MSG_CMN_HDR);

    if(imagesize > 0)
        memcpy(ptr, sendbuff.data(), sendbuff.size());

	msgLen = pSocket->sendData((const unsigned char*)pMsg, msgLen);

    delete[] pMsg;

    return msgLen;
}

unsigned int CommandEncoder::EncodeResponse(unsigned char* pMsg, unsigned int seqNum, unsigned int result)
{
	unsigned int msgLen = sizeof(S_MSG_CMN_HDR);

	memset(pMsg, 0x00, sizeof(S_MSG_CMN_HDR));

	// Header
	S_MSG_CMN_HDR* pstMsgHdr = (S_MSG_CMN_HDR*)pMsg;

	pstMsgHdr->msgType = htons(MSG_TYPE_RESPONSE);
	pstMsgHdr->mode = htons(result);
	pstMsgHdr->msgId = htonl(seqNum);
	pstMsgHdr->dataLen = 0;

	return msgLen;
}

//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------


