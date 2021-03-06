// camera.cpp
// Control various functions using camera.
// Executed by child process



// Get the frame from the camera.
// Create a mask that produces a movable part in this frame.
// The mask generated by this work gives an ROI on the areas of the road where the movement is made.
// After this, the frame is obtained only by passing the images through the mask, and the detector detects this area with each thread.



#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/bgsegm.hpp>
#include <iostream>
#include <thread>

#include "BackgroundMask.h"
#include "Detectors.h"
#include "Situation.h"
#include "CamDef.h"

using namespace cv; // openCV




// Detect objects by each detectors.
// calls detect () overridden on each detector.
// param - detector : This detector has learned the characteristics of the objects to be detected.
// param - fgimg :  (Forground images) Detection of objects in this image is detected, and the detection results are also shown in this image.
// ( The pedestrian is indicated by the green color, and the vehicle is red. )
void detectObjects(Detector& detector, UMat& fgimg) {
    detector.detect(fgimg);
}




//It is called from main().
// Performs overall operations using the camera.
int takeRoad(std::string videoSource)
{
    UMat img, fgimg; // using OpenCL ( UMat )
    // Import pre-learned shape recognition data
    PedestriansDetector pe_Detector;
    VehiclesDetector car_Detector;


    // Select the source of the video.
    VideoCapture vc;
    if ( videoSource == "CAMERA" ) {
        /// Connect camera
        vc.open(0);
        vc.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        vc.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        vc.set(CV_CAP_PROP_FPS, 12);
    } else {
        // Load test video
        vc.open( videoSource );
    }
    if (!vc.isOpened()) {
        std::cerr << "ERROR : Cannot open video source : " << videoSource << std::endl;
        return false;
    }



    // Background recognition and removal
    BackgroundMask bgMask;
    bgMask.setRecognizeNumFrames(24);  // Default : 120 ( BackgroundSubtractorGMG's default value )
    bgMask.setNoiseRemovalNumFrames( vc.get(CV_CAP_PROP_FPS) ); // Default : 12
    bgMask.setAccumulateNumFrames(300); // Default : 600
    bgMask.setLearningRate(0.025); // Default : 0.025
    // bgMask.printProperties();

    // Select the source of the mask.
    // UMat mask = bgMask.createBackgroundMask(vc);
    UMat mask = bgMask.loadBackgroundMask();
    imshow( CamDef::mask, mask );  // show background mask



    // Update the road image
    Situation situation( vc.get(CV_CAP_PROP_FRAME_HEIGHT), vc.get(CV_CAP_PROP_FRAME_WIDTH), vc.get(CV_CAP_PROP_FPS)*4 );

    // Select the source of the roadImg.
    // situation.createRoadImg(vc, bgMask, car_Detector, 300);
    situation.loadRoadImg();
    imshow( CamDef::roadImg, situation.getRoadImg() );
    // situation.setSignToFullScreen();



    std::cout << "Start Detection ..." << std::endl;
    bool isDetecting = true;
    while ( isDetecting ) {
        // Put the captured image in img
        vc >> img;
        if (img.empty())  {
            std::cerr << "ERROR : Unable to load frame" << std::endl;
            break;
        }
        imshow( CamDef::originalVideo, img );


        // Exclude areas excluding road areas in the original image.
        bgMask.locateForeground(img, fgimg);

        // Detect pedestrians and vehicle
        // Detect pedestrians and vehicle
        std::thread t1(detectObjects, std::ref(pe_Detector), std::ref(fgimg));
        std::thread t2(detectObjects, std::ref(car_Detector), std::ref(fgimg));
        t1.join();
        t2.join();


        // Judge the situation of the road
        situation.sendPredictedSituation( pe_Detector.getFoundObjects(), car_Detector.isFound() );


        // show image processing result
        imshow( CamDef::resultVideo, fgimg );
        if( CamDef::shouldStop() ) isDetecting = false;
    }


    // Return resources.
    std::cout << "Disconnecting from camera and returning resources ..." << std::endl;
    destroyAllWindows();
    img.release();
    fgimg.release();
    mask.release();
    vc.release();

    return true;
}
