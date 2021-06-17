#include <iostream>
#include <string>
#include <chrono>
#include <NvInfer.h>
#include <NvInferPlugin.h>
#include <l2norm_helper.h>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudawarping.hpp>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "faceNet.h"
#include "videoStreamer.h"
#include "network.h"
#include "mtcnn.h"

#include "NetworkTCP.h"
#include "TcpSendRecvJpeg.h"
#include <termios.h>
#include "crypto_op.h"
#include "authenticator.h"
#include "gssSocket.hpp"
#include "sslManager.h"
#include "CommandInterface.h"
#include "CommandEncoder.h"
#include "CommandDecoder.h"
#include "Logger.h"
#include "mode.h"

#define TEST_VIDEO_FILENAME         "../friends640x480.mp4"

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}
// Uncomment to print timings in milliseconds
// #define LOG_TIMES

using namespace std;
using namespace nvinfer1;
using namespace nvuffparser;

int Handle_RecvLoginRequest(int mode, unsigned char* pRcvMsg)
{
    int result = MODE_RESULT_OK;
    int idLen = 0;
    int pwdLen = 0;
    unsigned char* idBuf = NULL;
    unsigned char* pwdBuf = NULL;
    CommandDecoder decoder;

    unsigned char* ptr = pRcvMsg + sizeof(S_MSG_CMN_HDR);

    AccessRight accessRight = ACCESS_UNKNOWN;

    printf("Receive Login Request: 0x%x\n", mode);

    idLen = decoder.ParsingBuffer(ptr, &idBuf);
    ptr += sizeof(int) + idLen;

    pwdLen = decoder.ParsingBuffer(ptr, &pwdBuf);
    ptr += sizeof(int) + pwdLen;

    int idBufLen    = strlen((char *)idBuf);
    int pwdBufLen   = strlen((char *)pwdBuf);

    if(!authenticateUser(idBuf, idBufLen, pwdBuf, pwdBufLen, accessRight)) {
        result = MODE_RESULT_NOK;
    }
    else {
        switch(mode)
        {
            case MODE_LEARNING:
            case MODE_TESTRUN:
                if (accessRight != ACCESS_ADMIN)
                {
                    LOGGER->Log("Improper access right");
                    result = MODE_RESULT_NOK;
                }
                break;
            case MODE_RUN:
                break;
            default:
                printf("Invalid Mode: 0x%x\n", mode);
                result = MODE_RESULT_NOK;
                break;
        }
    }

    delete[] idBuf;
    delete[] pwdBuf;

    return result;
}

int Handle_RecvCaptureRequest(unsigned char* pRcvMsg)
{
    int result = 0;

    LOGGER->Log("Recv Capture Request");

    return MODE_RESULT_OK;
}

int Handle_RecvCancelRequest(unsigned char* pRcvMsg)
{
    int result = 0;

    LOGGER->Log("Recv Cancel Request");

    return MODE_RESULT_OK;
}

int Handle_RecvConfirmRequest(unsigned char* pRcvMsg, std::string &name4learn)
{
    int result = 0;
    int nameLen = 0;
    unsigned char* nameBuf = NULL;
    CommandDecoder decoder;

    unsigned char* ptr = pRcvMsg + sizeof(S_MSG_CMN_HDR);

    LOGGER->Log("Recv Confirm Request");

    nameLen = decoder.ParsingBuffer(ptr, &nameBuf);

    if (nameLen > 0 && IsValidString((const char *)nameBuf)) {
        name4learn = (std::string)(char *)nameBuf;
    }
    else {
        LOGGER->Log("Invalid name is delivered");
    }

    delete[] nameBuf;
    nameBuf = NULL;

    return MODE_RESULT_OK;
}

int onHandleRecvMessage(gssSocketClass *pSocket, int &nSelectedMode, int &nLearningMode, std::string &name4learn)
{
    int nHdrLen, n;
    unsigned char recvBuf[1000], sendBuf[1000];
    S_MSG_CMN_HDR* pstMsgCmnHdr = NULL;
    int result = 0;
    CommandEncoder encoder;
    CommandDecoder decoder;

    printf("onHandleRecvMessage\n");

    nHdrLen = sizeof(S_MSG_CMN_HDR);
    printf("nHdrLen: %d\n", nHdrLen);

    if((n = pSocket->receiveData(recvBuf, nHdrLen)) < 0)
    {
        printf("Failure to read message header\n");
        return -1;
    }

    pstMsgCmnHdr = (S_MSG_CMN_HDR*)recvBuf;
    decoder.DecodeCommonHeader(pstMsgCmnHdr);

    // Receive the message body
    printf("dataLen: %d\n", pstMsgCmnHdr->dataLen);
    if((n = pSocket->receiveData(recvBuf + nHdrLen, pstMsgCmnHdr->dataLen)) < 0)
    {
        printf("Failure to read message body\n");
        return -1;
    }

    if(pstMsgCmnHdr->msgType != MSG_TYPE_REQUEST)
    {
        LOGGER->Log("Invalid Message Type: 0x%x", pstMsgCmnHdr->msgType);
        return -1;
    }

    switch(pstMsgCmnHdr->mode)
    {
        case MODE_LEARNING: // FALL THROUGH
        case MODE_RUN:      // FALL THROUGH
        case MODE_TESTRUN:
            if((result = Handle_RecvLoginRequest(pstMsgCmnHdr->mode, recvBuf)) == MODE_RESULT_OK) {
                nSelectedMode   = pstMsgCmnHdr->mode;
            }
            break;
        case MODE_CAPTURE:
            if (nSelectedMode == MODE_LEARNING && ((result = Handle_RecvCaptureRequest(recvBuf)) == MODE_RESULT_OK)) {
                nLearningMode   = MODE_CAPTURE;     // Capture Start
            }
            break;
        case MODE_CANCEL:
            if (nSelectedMode == MODE_LEARNING && ((result = Handle_RecvCancelRequest(recvBuf)) == MODE_RESULT_OK)) {
                nLearningMode   = MODE_CANCEL;     // Capture Cancel
            }
            break;
        case MODE_CONFIRM:
            if (nSelectedMode == MODE_LEARNING && ((result = Handle_RecvConfirmRequest(recvBuf, name4learn)) == MODE_RESULT_OK)) {
                nLearningMode   = MODE_CONFIRM;     // Capture Confirm
            }
            break;
        default:
            LOGGER->Log("Invalid Mode: 0x%x", pstMsgCmnHdr->mode);
            break;
    }

    // Send OK or NOK
    int msgLen  = encoder.EncodeResponse(sendBuf, pstMsgCmnHdr->msgId, result);
    int result2 = pSocket->sendData(sendBuf, msgLen);

    return(result == MODE_RESULT_OK && result2 == msgLen);
}



void draw_facebox(cv::Mat &image, std::vector<Bbox> outBox)
{
    if (outBox.size() > 0) {
        cv::rectangle(image, cv::Point(outBox[0].y1, outBox[0].x1), cv::Point(outBox[0].y2, outBox[0].x2), cv::Scalar(0,0,255), 2,8,0);
    }
    /*
    for(vector<struct Bbox>::iterator it=outBox.begin(); it!=outBox.end();it++) {
        cv::rectangle(image, cv::Point((*it).y1, (*it).x1), cv::Point((*it).y2, (*it).x2), cv::Scalar(0,0,255), 2,8,0);
    }
    */
}


int main(int argc, char *argv[])
{
    int                portNum = 0;
    bool               bSecureMode = false;
    const char        *p_sVideoFileName = TEST_VIDEO_FILENAME;
    gssSocketClass     gssSocket;
    sslManagerClass    sslManager;
    CommandEncoder     encoder;

    int recvBufLen, sendMessageLen;

    if(!canFileOpen(p_sVideoFileName)) {
        LOGGER->Log("test-video file cannot be open");
        exit(0);
    }

    if(!checkCmdArguments(argc, argv, portNum, bSecureMode)) {
        fprintf(stderr, "usage %s [port] [1|0 (secured or not)]\n", argv[0]);
        exit(0);
    }

    if(!bSecureMode)
        LOGGER->Log("Start running as Non-Secure mode");
    else
        LOGGER->Log("Start running as Secure mode");

    if(!authenticateSystem())
        exit(0);

    Logger gLogger = Logger();

    // Register default TRT plugins (e.g. LRelu_TRT)
    if (!initLibNvInferPlugins(&gLogger, "")) { return 1; }

    // USER DEFINED VALUES
    const string uffFile = "../facenetModels/facenet.uff";
    const string engineFile = "../facenetModels/facenet.engine";
    DataType dtype = DataType::kHALF;
    //DataType dtype = DataType::kFLOAT;
    bool serializeEngine = true;
    int batchSize = 1;
    int nbFrames = 0;
    int videoFrameWidth  = 640;
    int videoFrameHeight = 480;

    int maxFacesPerScene = 8;
    float knownPersonThreshold = 1.;
    bool isCSICam = true;

    // init facenet
    FaceNetClassifier faceNet = FaceNetClassifier(gLogger, dtype, uffFile, engineFile, batchSize, serializeEngine,
            knownPersonThreshold, maxFacesPerScene, videoFrameWidth, videoFrameHeight);

    VideoStreamer *videoCameraStreamer = NULL;
    VideoStreamer *videoFileStreamer   = NULL;

    cv::Mat frame;
    cv::Mat frame_captured;         // For learning, save the current face image
    cv::Mat frame_captured_boxed;   // In learning mode, face-boxed image with frame_captured

    // init mtCNN
    mtcnn mtCNN(videoFrameHeight, videoFrameWidth);

    //init Bbox and allocate memory for "maxFacesPerScene" faces per scene
    std::vector<struct Bbox> outputBbox;
    outputBbox.reserve(maxFacesPerScene);

    // get embeddings of known faces
    std::vector<struct Paths> paths;
    cv::Mat image;

/*  Only use to encrypt existed image files.
    getFilePaths("../imgs", paths);
    std::cout << "path size : " << paths.size() << std::endl;
    std::string dirPath = "../imgs/";
    for(int i = 0; i < paths.size(); i++)
    {
        encrypt_file(dirPath.c_str(), dirPath.length(),
                    paths[i].fileName.c_str(), paths[i].fileName.length());
    }

    paths.clear();
*/

    getEncryptedFilePaths("../imgs", paths);

    for(int i = 0; i < paths.size(); i++)
    {
        std::string name;
        if (!loadInputImage(paths[i].absPath, image, name, videoFrameWidth, videoFrameHeight))
        {
            cout << "loadInputImage failed " << endl;
            exit(-1);
        }

        outputBbox = mtCNN.findFace(image);
        faceNet.forwardAddFace(image, outputBbox, name);
        faceNet.resetVariables();
    }

    outputBbox.clear();

    //printf("---------------------\n");

    /* If the camera is CSICam, it must be always open */
    if(isCSICam) {
        //videoCameraStreamer = openCameraStream(videoFrameWidth, videoFrameHeight, true);
	videoCameraStreamer = new VideoStreamer(0, videoFrameWidth, videoFrameHeight, 60, isCSICam);
        if(videoCameraStreamer == NULL) {
            LOGGER->Log("CSI Camera cannot be open\n");
            //exit(-1);    // This comment must be removed for the official release
        }
	else {
            LOGGER->Log("CSI Camera open\n");
	}
    }

    /* OpenSSL Initialization */
    initOpenSSL();
    sslManager.setRole(true);       // Work as TLS server (in cas of TLS client, set false)
    if(sslManager.loadKeyCertificate() == 0) {
        LOGGER->Log("Security key and certificate loading failure");
        exit(0);
    }
    LOGGER->Log("Security key and certificate loading Ok");

    /* Socket binding with OpenSSL, and open the listen socket*/
    gssSocket.bindSSL(sslManager.getSslCtx());
    if(!gssSocket.initListenSockets(portNum, bSecureMode)) {
        LOGGER->Log("Listen socket initialization failure\n");
        exit(0);
    }
    LOGGER->Log("Listening started");

    /* Initialization of CUDA */
    cv::cuda::GpuMat src_gpu, dst_gpu;
    cv::Mat dst_img;


    bool bTerminateLoop1 = false;
    while (!bTerminateLoop1) {
        if(gssSocket.waitListenSocketAccept()) {
            LOGGER->Log("Connection accepted");

            bool bSendStreamData = false;
            int  nSelectedMode   = -1;
            int  nLearningMode   = -1;
            bool bCaptured       = false;
            std::string sName4Learn;

            videoFileStreamer    = NULL;
            if(!isCSICam) videoCameraStreamer = NULL;

            bool bTerminateLoop2 = false;
            while(!bTerminateLoop2) {
                int k = gssSocket.isReceived();
                if(k < 0) {
                    bTerminateLoop2 = true;
                    LOGGER->Log("Network Connection Failure. Disconnected!!");
                }
                else if (k > 0) {
                    printf("Before Message Receive\n");
                    int nTempSelectedMode = nSelectedMode;
                    int nTempLearningMode = nLearningMode;
                    if(!onHandleRecvMessage(&gssSocket, nTempSelectedMode, nTempLearningMode, sName4Learn)) {
                        bTerminateLoop2 = true;
                    }
                    else {
                        /* To-do for each mode */
                        if (nSelectedMode != nTempSelectedMode && nTempSelectedMode > 0) {
                            nSelectedMode = nTempSelectedMode;
                            switch(nSelectedMode) {
                            case MODE_RUN:  // RUN mode
                                if(!startMode_RUN(bSendStreamData, &videoCameraStreamer, videoFrameWidth,
                                                    videoFrameHeight, isCSICam)) {
                                    bTerminateLoop2 = true;
                                }
                                else {
                                    LOGGER->Log("RUN mode starts");
                                }
                                break;
                            case MODE_TESTRUN:  // TEST RUN mode
                                if(!startMode_TEST(bSendStreamData, &videoFileStreamer, videoFrameWidth,
                                                    videoFrameHeight, p_sVideoFileName)) {
                                    bTerminateLoop2 = true;
                                }
                                else {
                                    LOGGER->Log("TEST RUN mode starts");
                                }
                                break;
                            case MODE_LEARNING: // LEARNING mode
                                if(!startMode_LEARN(bSendStreamData, &videoCameraStreamer, videoFrameWidth,
                                                    videoFrameHeight, isCSICam)) {
                                    LOGGER->Log("LEARNING mode start failure: Camera cannot be open");
                                    bTerminateLoop2 = true;
                                }
                                else {
                                    LOGGER->Log("LEARNING mode starts");
                                }
                                break;
                            default:
                                bTerminateLoop2 = true;
                                break;
                            }
                        }
                        /* To-do for Request/Cancel/Confirm in Learning Mode */
                        else if(nSelectedMode == MODE_LEARNING && nTempLearningMode > 0) {
                            nLearningMode = nTempLearningMode;
                        }
                    }
                    printf("After Message Receive\n");
                }
                else {
                    // If you want to do something without recv message,
                    //onSendResponse_Test1(&gssSocket);
                }

                //bSendStreamData = true;
                if (bTerminateLoop2 == false && bSendStreamData) {
                    // On test-run mode, at the end of video, exceptio code needed
                    if(nSelectedMode == MODE_LEARNING || nSelectedMode == MODE_RUN)
                        videoCameraStreamer->getFrame(frame);
                    else
                        videoFileStreamer->getFrame(frame);

                    if (frame.empty()) {
                        if (nSelectedMode == MODE_TESTRUN) {
                            videoFileStreamer->release();
                            delete videoFileStreamer;
                            LOGGER->Log("Video file is replaying");
                            if((videoFileStreamer = openVideoFileStream(videoFrameWidth, videoFrameHeight, p_sVideoFileName)) == NULL) {
                                LOGGER->Log("Video file stream cannot be open");
                                bTerminateLoop2 = true;
                                break;
                            }
                            else {
                                videoFileStreamer->getFrame(frame);
                            }
                        }
                        else {
                            LOGGER->Log("Camera stream can't be open");
                            bTerminateLoop2 = true;
                            break;
                        }
                    }

                    // Create a destination to paint the source into.
                    dst_img.create(frame.size(), frame.type());

                    // Push the images into the GPU
                    if (isCSICam == true && (nSelectedMode == MODE_RUN || nSelectedMode == MODE_LEARNING)) {
                        //printf("Rotated\n");
                        src_gpu.upload(frame);
                        cv::cuda::rotate(src_gpu, dst_gpu, src_gpu.size(), 180, src_gpu.size().width, src_gpu.size().height);
                        dst_gpu.download(frame);
                    }

                    if (nSelectedMode != MODE_LEARNING) {   // RUN mode & TEST mode
                        outputBbox = mtCNN.findFace(frame);
                        faceNet.forward(frame, outputBbox);
                        faceNet.featureMatching(frame);
                        faceNet.resetVariables();

                        if(encoder.SendStreamData(&gssSocket, frame) < 0) {
                            LOGGER->Log("Cannot Send Stream Data");
                            bTerminateLoop2 = true;
                        }
                    }
                    else {      // LEARNING mode
                        switch(nLearningMode) {
                        case MODE_CAPTURE:
                            // Try to capture
                            if (bCaptured == false) {
                                outputBbox = mtCNN.findFace(frame);
                                if(outputBbox.size() > 0) {
                                    frame_captured       = frame.clone();
                                    frame_captured_boxed = frame_captured.clone();
                                    draw_facebox(frame_captured_boxed, outputBbox);
                                    //cv::imshow("FrameCaptued", frame_captured_boxed);

                                    if(encoder.SendStreamData(&gssSocket, frame_captured_boxed) < 0) {
                                        LOGGER->Log("Cannot Send Captured Data");
                                        bTerminateLoop2 = true;
                                    }
                                    else {
                                        bCaptured = true;
                                    }
                                }
                            }
                            break;
                        case MODE_CANCEL:
                            bCaptured       = false;
                            break;
                        case MODE_CONFIRM:
                            // Try to learn with frame_captured
                            if (bCaptured == true) {
                                // Call Learning
                                outputBbox = mtCNN.findFace(frame_captured);
                                faceNet.addNewFaceWithName(frame_captured, outputBbox, sName4Learn);
                                bCaptured   = false;
                            }
                            break;
                        default:
                            break;
                        }

                        if(!bCaptured) {
                            if(encoder.SendStreamData(&gssSocket, frame) < 0) {
                                LOGGER->Log("Cannot Send Stream Data");
                                bTerminateLoop2 = true;
                            }
                        }
                    }

                    nbFrames++;
                    outputBbox.clear();
                    frame.release();
                }
            }  // bTerminateLoope2 - During Read/Write Socket data

            if (isCSICam == false && videoCameraStreamer != NULL) {
                videoCameraStreamer->release();
                delete videoCameraStreamer;
                videoCameraStreamer = NULL;
            }
            if(videoFileStreamer != NULL) {
                videoFileStreamer->release();
                delete videoFileStreamer;
                videoFileStreamer = NULL;
            }

            LOGGER->Log("Connection closed");
        }   // Wait Accept Listen Socket
    }   // bTerminateLoop1 - Infinite loop

    if(isCSICam == true && videoCameraStreamer !=NULL) {
        videoCameraStreamer->release();
        delete videoCameraStreamer;
        videoCameraStreamer = NULL;
    }

    return 0;
}



#if 0
    while (true) {
        videoStreamer->getFrame(frame);
        if (frame.empty()) {
            std::cout << "Empty frame! Exiting...\n Try restarting nvargus-daemon by "
                         "doing: sudo systemctl restart nvargus-daemon" << std::endl;
            break;
        }
        // Create a destination to paint the source into.
        dst_img.create(frame.size(), frame.type());

        // Push the images into the GPU
        if (UseCamera)
        {
            src_gpu.upload(frame);
            cv::cuda::rotate(src_gpu, dst_gpu, src_gpu.size(), 180, src_gpu.size().width, src_gpu.size().height);
            dst_gpu.download(frame);
        }

        auto startMTCNN = chrono::steady_clock::now();
        outputBbox = mtCNN.findFace(frame);
        auto endMTCNN = chrono::steady_clock::now();
        auto startForward = chrono::steady_clock::now();
        faceNet.forward(frame, outputBbox);
        auto endForward = chrono::steady_clock::now();
        auto startFeatM = chrono::steady_clock::now();
        faceNet.featureMatching(frame);
        auto endFeatM = chrono::steady_clock::now();
        faceNet.resetVariables();

        //if (TcpSendImageAsJpeg(TcpConnectedPort,frame) < 0)  break;
        if (SendStreamData(TcpConnectedPort, frame) < 0)  break;
        //cv::imshow("VideoSource", frame);
        nbFrames++;
        outputBbox.clear();
        frame.release();
        if (kbhit())
        {
        // Stores the pressed key in ch
            char keyboard =  getch();

            if (keyboard == 'q') break;
            else if(keyboard == 'n')
            {
                auto dTimeStart = chrono::steady_clock::now();
                videoStreamer->getFrame(frame);
                // Create a destination to paint the source into.
                dst_img.create(frame.size(), frame.type());

                // Push the images into the GPU
                src_gpu.upload(frame);
                cv::cuda::rotate(src_gpu, dst_gpu, src_gpu.size(), 180, src_gpu.size().width, src_gpu.size().height);
                dst_gpu.download(frame);

                outputBbox = mtCNN.findFace(frame);
                if (TcpSendImageAsJpeg(TcpConnectedPort,frame)<0)  break;
                //cv::imshow("VideoSource", frame);
                faceNet.addNewFace(frame, outputBbox);
                auto dTimeEnd = chrono::steady_clock::now();
                globalTimeStart += (dTimeEnd - dTimeStart);
            }
        }
    }

    auto globalTimeEnd = chrono::steady_clock::now();
    videoStreamer->release();

    return 0;
}

#endif  // #if 0
