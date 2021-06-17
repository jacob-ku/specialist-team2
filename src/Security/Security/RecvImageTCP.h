//------------------------------------------------------------------------------------------------
// File: RecvImageTCP.h
// Project: LG Exec Ed Program
// Versions:
// 1.0 April 2021 - initial version
// Provides the ability to receive jpeg images
//------------------------------------------------------------------------------------------------
#ifndef RecvImageTCPH
#define RecvImageTCPH
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

//int RecvImageTCP(const char* pIpAddr, const char* pPortNum);
int RecvImageTCP(const char* pIpAddr, const char* pPortNum, Mat* Image);

#endif
//------------------------------------------------------------------------------------------------
//END of Include
//------------------------------------------------------------------------------------------------
