//------------------------------------------------------------------------------------------------
// File: MessageEncoder.cpp
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to encode packets
//------------------------------------------------------------------------------------------------


#include "CommandInterface.h"
#include "CommandEncoder.h"

unsigned int CommandEncoder::SendConnectReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum, unsigned int mode,
	unsigned int idLen, char* pIdBuf, unsigned int pwdLen, char* pPwdBuf)
{
	idLen += 1;		// \0
	pwdLen += 1;	// \0

	unsigned int hdrLen = sizeof(S_MSG_CMN_HDR);
	unsigned int bodyLen = sizeof(idLen) + idLen + sizeof(pwdLen) + pwdLen;
	unsigned int totalLen = hdrLen + bodyLen;

	*pMsg = new char[totalLen];
	if (*pMsg == NULL)
		return -1;

	char* ptr = *pMsg;
	memset(*pMsg, 0x00, totalLen);

	// Header
	S_MSG_CMN_HDR* pstMsgHdr = (S_MSG_CMN_HDR*)(*pMsg);

	pstMsgHdr->msgType = htons(MSG_TYPE_REQUEST);
	pstMsgHdr->mode = htons(mode);
	pstMsgHdr->msgId = htonl(seqNum); 
	pstMsgHdr->dataLen = htonl(bodyLen);
	
	ptr += hdrLen;

	// Payload
	// Id Length
	unsigned int* pMsgIdLen = (unsigned int*)ptr;
	*pMsgIdLen = htonl(idLen);
	ptr += sizeof(int);

	// ID
	char* pMsgIdBuf = (char*)ptr;
	if (idLen > 0)
	{
		memcpy_s(pMsgIdBuf, idLen, pIdBuf, idLen);
		pMsgIdBuf[idLen - 1] = '\0';
	}
		
	ptr += idLen;

	// PWD Length
	unsigned int* pMsgPwLen = (unsigned int*)ptr;
	*pMsgPwLen = htonl(pwdLen);
	ptr += sizeof(int);

	// PWD
	char* pMsgPwdBuf = (char*)ptr;
	if (pwdLen > 0)
	{
		memcpy_s(pMsgPwdBuf, pwdLen, pPwdBuf, pwdLen); 
		pMsgPwdBuf[pwdLen - 1] = '\0';
	}
		
	ptr += pwdLen;

	// Send Message
	if (secure == 0) // Non Secure
	{
		send(socket, (*pMsg), totalLen, 0);
	}
	else
	{
		WriteDataTls(ssl, (unsigned char*)(*pMsg), totalLen);
	}

	seqNum++;

	return totalLen;
}

unsigned int CommandEncoder::SendLearnCaptureReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum)
{
	unsigned int msgLen = sizeof(S_MSG_CMN_HDR);

	*pMsg = new char[msgLen];
	if (*pMsg == NULL)
		return -1;

	char* ptr = *pMsg;
	memset(*pMsg, 0x00, msgLen);

	// Header
	S_MSG_CMN_HDR* pstMsgHdr = (S_MSG_CMN_HDR*)(*pMsg);

	pstMsgHdr->msgType = htons(MSG_TYPE_REQUEST);
	pstMsgHdr->mode = htons(MODE_CAPTURE);
	pstMsgHdr->msgId = htonl(seqNum);
	pstMsgHdr->dataLen = 0;

	// Send Message
	if (secure == 0) // Non Secure
	{
		send(socket, (*pMsg), msgLen, 0);
	}
	else
	{
		WriteDataTls(ssl, (unsigned char*)(*pMsg), msgLen);
	}

	seqNum++;

	return msgLen;
}

unsigned int CommandEncoder::SendLearnCancelReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum)
{
	unsigned int msgLen = sizeof(S_MSG_CMN_HDR);

	*pMsg = new char[msgLen];
	if (*pMsg == NULL)
		return -1;

	char* ptr = *pMsg;
	memset(*pMsg, 0x00, msgLen);

	// Header
	S_MSG_CMN_HDR* pstMsgHdr = (S_MSG_CMN_HDR*)(*pMsg);

	pstMsgHdr->msgType = htons(MSG_TYPE_REQUEST);
	pstMsgHdr->mode = htons(MODE_CANCEL);
	pstMsgHdr->msgId = htonl(seqNum);
	pstMsgHdr->dataLen = 0;

	// Send Message
	if (secure == 0) // Non Secure
	{
		send(socket, (*pMsg), msgLen, 0);
	}
	else
	{
		WriteDataTls(ssl, (unsigned char*)(*pMsg), msgLen);
	}

	seqNum++;

	return msgLen;
}

unsigned int CommandEncoder::SendLearnConfirmReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum, unsigned int nameLen, char* pNameBuf)
{
	nameLen += 1;	// \0

	unsigned int dataLen = sizeof(unsigned int) + nameLen;
	unsigned int msgLen = sizeof(S_MSG_CMN_HDR) + dataLen;

	*pMsg = new char[msgLen];
	if (*pMsg == NULL)
		return -1;

	char* ptr = *pMsg;
	memset(*pMsg, 0x00, msgLen);

	// Header
	S_MSG_CMN_HDR* pstMsgHdr = (S_MSG_CMN_HDR*)(*pMsg);

	pstMsgHdr->msgType = htons(MSG_TYPE_REQUEST);
	pstMsgHdr->mode = htons(MODE_CONFIRM);
	pstMsgHdr->msgId = htonl(seqNum);
	pstMsgHdr->dataLen = htonl(dataLen);
	
	ptr += sizeof(S_MSG_CMN_HDR);

	// Payload
	// Name Length
	unsigned int* pMsgNameLen = (unsigned int*)ptr;
	*pMsgNameLen = htonl(nameLen);
	ptr += sizeof(int);

	// Name
	char* pMsgNameBuf = (char*)ptr;
	if (nameLen > 0)
	{
		memcpy_s(pMsgNameBuf, nameLen, pNameBuf, nameLen);
		pMsgNameBuf[nameLen - 1] = '\0';
	}
		
	ptr += nameLen;

	// Send Message
	if (secure == 0) // Non Secure
	{
		send(socket, (*pMsg), msgLen, 0);
	}
	else
	{
		WriteDataTls(ssl, (unsigned char*)(*pMsg), msgLen);
	}

	seqNum++;

	return msgLen;
}

//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------


