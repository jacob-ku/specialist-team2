//------------------------------------------------------------------------------------------------
// File: CommandDecoder.h
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to decode packets
//------------------------------------------------------------------------------------------------
#pragma once
#include "CommandInterface.h"
#include "CommandDecoder.h"
#include "gssSocket.hpp"

class CommandDecoder {
private:

public:
	void DecodeCommonHeader(S_MSG_CMN_HDR *pstMsgCmnHdr);
    int ParsingBuffer(unsigned char* pRcvMsg, unsigned char** buf);
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


