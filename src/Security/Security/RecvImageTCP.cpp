//------------------------------------------------------------------------------------------------
// File: RecvImageTCP.cpp
// Project: LG Exec Ed Program
// Versions:
// 1.0 April 2017 - initial version
// This program receives a jpeg image via a TCP Stream and displays it. 
//----------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "NetworkTCP.h"
#include "TcpSendRecvJpeg.h"
#include "RecvImageTCP.h"


using namespace cv;
using namespace std;
//----------------------------------------------------------------
// RecvImageTCP - This is the main program for the RecvImageTCP  
// program  contains the control loop
//-----------------------------------------------------------------
#if 0
int RecvImageTCP(const char* pIpAddr, const char* pPortNum)
{
	TTcpConnectedPort* TcpConnectedPort = NULL;
	bool retvalue;

	// TODO: Need to check Validation of ipAddr and portNum

	if ((TcpConnectedPort = OpenTcpConnection(pIpAddr, pPortNum)) == NULL)  // Open TCP Network port
	{
		printf("OpenTcpConnection\n");
		return(-1);
	}

	namedWindow("Server", WINDOW_AUTOSIZE);// Create a window for display.

	Mat Image;

	do {
		retvalue = TcpRecvImageAsJpeg(TcpConnectedPort, &Image);

		if (retvalue)
		{
			
			imshow("Server", Image); // If a valid image is received then display it
		}
		else
		{
			break;
		}

	} while (waitKey(10) != 'q'); // loop until user hits quit

	CloseTcpConnectedPort(&TcpConnectedPort); // Close network port;
	return 0;
}
#else
int RecvImageTCP(const char* pIpAddr, const char* pPortNum, Mat* Image)
{
	TTcpConnectedPort* TcpConnectedPort = NULL;
	bool retvalue;

	// TODO: Need to check Validation of ipAddr and portNum

	if ((TcpConnectedPort = OpenTcpConnection(pIpAddr, pPortNum)) == NULL)  // Open TCP Network port
	{
		printf("OpenTcpConnection\n");
		return(-1);
	}

	//namedWindow("Server", WINDOW_AUTOSIZE);// Create a window for display.

	

	
	retvalue = TcpRecvImageAsJpeg(TcpConnectedPort, Image);

	if (retvalue)
	{

		//imshow("Server", Image); // If a valid image is received then display it
	}

	return 0;
}
#endif
//-----------------------------------------------------------------
// RecvImageTCP main
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------
