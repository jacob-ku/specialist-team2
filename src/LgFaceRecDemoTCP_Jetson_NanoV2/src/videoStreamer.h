#ifndef VIDEO_INPUT_WRAPPER_VIDEOSTREAMER_H
#define VIDEO_INPUT_WRAPPER_VIDEOSTREAMER_H

#include <iostream>
#include <assert.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>



class VideoStreamer {
private:
    int m_videoWidth;
    int m_videoHeight;
	int m_frameRate;
    cv::VideoCapture *m_capture;

public:
    VideoStreamer(int nmbrDevice, int videoWidth, int videoHeight, int frameRate, bool isCSICam);
    VideoStreamer(std::string filename, int videoWidth, int videoHeight);
    ~VideoStreamer();
    void setResolutionDevice(int width, int height);
    void setResoltionFile(int width, int height);
    void assertResolution();
    void getFrame(cv::Mat &frame);
	std::string gstreamer_pipeline (int capture_width, int capture_height, int display_width, int 	display_height, int frameRate, int flip_method=0);
    void release();
};

VideoStreamer *openCameraStream(
                    int videoFrameWidth,
                    int videoFrameHeight,
                    bool isCSICam
                    );

VideoStreamer *openVideoFileStream(
                    int videoFrameWidth,
                    int videoFrameHeight,
                    const char *pFileName
                    );

#endif //VIDEO_INPUT_WRAPPER_VIDEOSTREAMER_H
