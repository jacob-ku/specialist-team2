//------------------------------------------------------------------------------------------------
// File: CommandEncoder.h
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to encode packets
//------------------------------------------------------------------------------------------------
#pragma once
#include <cstring>
#if  defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else

#endif
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "NetworkTLS.h"

class CommandEncoder
{
private:


public:
	unsigned int SendConnectReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum, unsigned int mode, unsigned int idLen, char* pIdBuf, unsigned int pwdLen, char* pPwdBuf);
	unsigned int SendLearnCaptureReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum);
	unsigned int SendLearnCancelReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum);
	unsigned int SendLearnConfirmReq(int secure, SOCKET socket, SSL* ssl, char** pMsg, unsigned int& seqNum, unsigned int nameLen, char* pName);
};

//------------------------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
//  Function Prototypes 
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//END of Include
//------------------------------------------------------------------------------------------------


