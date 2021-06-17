#ifndef __MODE_HEADER__
#define __MODE_HEADER__

bool startMode_RUN(
                    bool &bSendStream, 
                    VideoStreamer **vStreamer,
                    int videoFrameWidth,
                    int videoFrameHeight,
                    bool isCSICam
                    );

bool startMode_TEST(
                    bool &bSendStream, 
                    VideoStreamer **vStreamer,
                    int videoFrameWidth,
                    int videoFrameHeight,
                    const char *pVideoFile
                    );

bool startMode_LEARN(
                    bool &bSendStream, 
                    VideoStreamer **vStreamer,
                    int videoFrameWidth,
                    int videoFrameHeight,
                    bool isCSICam
                    );

#endif