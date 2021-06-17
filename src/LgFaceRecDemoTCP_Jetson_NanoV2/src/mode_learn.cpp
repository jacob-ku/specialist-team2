#include "videoStreamer.h"
#include "Logger.h"

bool startMode_LEARN(
                    bool &bSendStreamData, 
                    VideoStreamer **vStreamer,
                    int videoFrameWidth,
                    int videoFrameHeight,
                    bool isCSICam
                    )
{
    int ret;

    if (isCSICam == true) {
        bSendStreamData = true;
        ret = true;
    }
    else {
        VideoStreamer *v = openCameraStream(videoFrameWidth, videoFrameHeight, isCSICam);
        if(v != NULL) {
            bSendStreamData = true;
            (*vStreamer)    = v;
            ret = true;
        }
        else {
            bSendStreamData = false;
            (*vStreamer)    = NULL;
            ret = false;
            LOGGER->Log("LEARN mode start failure: Camera stream cannot be open");
        }
    }

    return ret;
}

