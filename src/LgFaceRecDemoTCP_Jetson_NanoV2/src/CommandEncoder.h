//------------------------------------------------------------------------------------------------
// File: CommandEncoder.h
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides the ability to encode packets
//------------------------------------------------------------------------------------------------
#pragma once
#include <opencv2/core/core.hpp>
#include "CommandInterface.h"
#include "CommandDecoder.h"
#include "gssSocket.hpp"

class CommandEncoder {
private:

public:
    int SendStreamData(gssSocketClass *pSocket, cv::Mat Image);
    unsigned int EncodeResponse(unsigned char* pMsg, unsigned int seqNum, unsigned int result);
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


