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
#include "CamDef.h"

#include "../communication/SigDef.h"

using namespace cv; // openCV





// Detect objects by each detectors.
// param - fgimg :  (Forground images) Detection of objects in this image is detected, and the detection results are also shown in this image.
// ( The pedestrian is indicated by the green color, and the vehicle is red. )
// param - detector : This detector has learned the characteristics of the objects to be detected.
// param - signo : Types of signals that generate signals for detection
void detectObjects(UMat& fgimg, Detector& detector, int signo) {
    // calls detect () overridden on each detector.
    detector.detect(fgimg);

    // Behavior when an object is found in the current frame
    if( detector.isFound() ) {
       sendSignalToParentProcess(signo);

        std::string objtype;
        if ( signo == SigDef::SIG_FOUND_HUMAN ) {
            objtype = "Human";
        } else {
            objtype = "Car";
        }

        // TODO : Store the coordinates for a period of time and predict the risk situation.
        // Outputs the coordinates of the found objects in frame.
        const std::vector<Rect>& foundObj = detector.getFoundObjects();
        for( auto const& r : foundObj ) {
            std::cout << objtype << " : tl = (" << r.tl().x << "," << r.tl().y << ") , br = ("
            << r.br().x << "," << r.br().y << "), md = ("
            << ( r.br().x - r.tl().x )/2 + r.tl().x << "," << ( r.br().y - r.tl().y )/2 + r.tl().y << ")" << std::endl;
        }
    }
}





//It is called from main().
// Performs overall operations using the camera.
int takeRoad(void)
{
    // Select the source of the video.

    /* // Connect camera
    VideoCapture vc(0);
    vc.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    vc.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    vc.set(CV_CAP_PROP_FPS, 12);
    */
    // Load test video
    VideoCapture vc( CamDef::sampleVideo );
    if (!vc.isOpened()) {
        std::cerr << "ERROR : Cannot open the camera" << std::endl;
        return false;
    }




    // Background recognition and removal
    BackgroundMask bgMask;
    bgMask.setRecognizeNumFrames(24);  // Default : 120 ( BackgroundSubtractorGMG's default value )
    bgMask.setNoiseRemovalNumFrames( vc.get(CV_CAP_PROP_FPS) ); // Default : 12
    bgMask.setAccumulateNumFrames(120); // Default : 600
    bgMask.setLearningRate(0.025); // Default : 0.025
    bgMask.printProperties();

    // Select the source of the mask.
    // UMat mask = bgMask.createBackgroundMask(vc);
    UMat mask = bgMask.roadBackgroundMask();
    imshow( CamDef::mask, mask );  // show background mask




    UMat img, fgimg; // using OpenCL ( UMat )
    PedestriansDetector pe_Detector;
    VehiclesDetector car_Detector;

    car_Detector.initRoadImg(mask);


    std::cout << "Start Detection ..." << std::endl;
    bool playVideo = true; char pressedKey;
    while (1) {
        if(playVideo) {
            // Put the captured image in img
            vc >> img;
            if (img.empty())  {
                std::cerr << "ERROR : Unable to load frame" << std::endl;
                break;
            }


            // Exclude areas excluding road areas in the original image.
            bgMask.locateForeground(img, fgimg);

            // Detect pedestrians and vehicle
            std::thread t1(detectObjects, std::ref(fgimg), std::ref(pe_Detector), SigDef::SIG_FOUND_HUMAN);
            std::thread t2(detectObjects, std::ref(fgimg), std::ref(car_Detector), SigDef::SIG_FOUND_CAR);
            t1.join();
            t2.join();
        }


        // Print out the images in the window.
        imshow( CamDef::originalVideo, img );  // show original image
        imshow( "roadImg", car_Detector.getRoadImg() );  // show background mask
        imshow( CamDef::resultVideo, fgimg );  // show image


        // press SPACE BAR -> pause video
        // press ESC -> close video
        if ( ( pressedKey = waitKey( CamDef::DELAY ) ) == CamDef::PAUSE ) // SPACE BAR
            playVideo = !playVideo;
        else if(  pressedKey == CamDef::CLOSE ) { // ESC
            std::cout << "Disconnecting from camera and returning resources ..." << std::endl;
            break;
        }
    }


    // Return resources.
    destroyAllWindows();
    img.release();
    fgimg.release();
    mask.release();
    vc.release();

    return true;
}
