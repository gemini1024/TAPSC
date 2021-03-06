// BackgroundMask.h
// recognize the background and create a frame that excludes this part
// For better detecting performance

#ifndef BACKGROUNDMASK_H
#define BACKGROUNDMASK_H

#include <opencv2/highgui.hpp>
#include <opencv2/bgsegm.hpp>

using namespace cv;



class BackgroundMask {
private :
    Ptr<bgsegm::BackgroundSubtractorGMG> pMask = bgsegm::createBackgroundSubtractorGMG();
    UMat bgMask; // Mask excluding moving objects
    UMat accumulatedMask;
    int accumulateNumFrames;
    int noiseRemovalNumFrames;

private :
    void recognizeBackgournd(VideoCapture& vc);
    void accumulateMasks(VideoCapture& vc);

public :
    BackgroundMask();
    ~BackgroundMask();
    UMat createBackgroundMask(VideoCapture& vc);
    UMat loadBackgroundMask(void);
    void printProperties(void);
    void setRecognizeNumFrames(int num);
    void setNoiseRemovalNumFrames(int num);
    void setAccumulateNumFrames(int num);
    void setLearningRate(double rate);
    void locateForeground(UMat& src, UMat& dst);
};


#endif