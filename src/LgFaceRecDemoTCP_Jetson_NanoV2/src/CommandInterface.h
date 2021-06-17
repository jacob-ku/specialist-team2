//------------------------------------------------------------------------------------------------
// File: CommandInterface.h
// Project: 
// Versions:
// 1.0 June 2021 - initial version
// Provides definition of message interface
//------------------------------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------------------------
// Definition
//------------------------------------------------------------------------------------------------
/* Message Type */
constexpr unsigned short MSG_TYPE_REQUEST		= (0x5501);
constexpr unsigned short MSG_TYPE_RESPONSE		= (0x5502);
constexpr unsigned short MSG_TYPE_STREAM		= (0x5503);

/* Mode */
constexpr unsigned short MODE_LEARNING			= (0x3301);
constexpr unsigned short MODE_RUN				= (0x3302);
constexpr unsigned short MODE_TESTRUN			= (0x3303);
constexpr unsigned short MODE_CAPTURE			= (0x3304);
constexpr unsigned short MODE_CANCEL			= (0x3305);
constexpr unsigned short MODE_CONFIRM			= (0x3306);
constexpr unsigned short MODE_RESULT_OK			= (0x3307);
constexpr unsigned short MODE_RESULT_NOK		= (0x3308);
constexpr unsigned short MODE_STREAM			= (0x3309);

constexpr unsigned int LEN_CMN_HDR	= (12);

typedef struct {
	unsigned short msgType;
	unsigned short mode;
	unsigned int msgId;
	unsigned int dataLen;
} S_MSG_CMN_HDR;

//------------------------------------------------------------------------------------------------
//  Function Prototypes 
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//END of Include
//------------------------------------------------------------------------------------------------


