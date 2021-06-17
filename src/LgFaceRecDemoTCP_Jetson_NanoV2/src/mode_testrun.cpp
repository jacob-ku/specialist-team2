#include "videoStreamer.h"
#include "Logger.h"

bool startMode_TEST(
                    bool &bSendStreamData, 
                    VideoStreamer **vStreamer,
                    int videoFrameWidth,
                    int videoFrameHeight,
                    const char *pFileName
                    )
{
    int ret;

    VideoStreamer *v = openVideoFileStream(videoFrameWidth, videoFrameHeight, pFileName);
    if(v != NULL) {
        bSendStreamData = true;
        (*vStreamer)    = v;
        ret = true;
    }
    else {
        bSendStreamData = false;
        (*vStreamer)    = NULL;
        ret = false;
        LOGGER->Log("TEST-RUN mode start failure: File stream annot be open");
    }

    return ret;
}
