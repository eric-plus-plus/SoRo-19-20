#include <vector>
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/flann.hpp>
#include <opencv2/aruco.hpp>
#include <unistd.h>

/*NOTE: Certain settings need adjusted before code is run. To do this run the code below:
    v4l2-ctl --list-devices #to find the device you want. Below I assume /dev/video1
    v4l2-ctl -d /dev/video1 --set-ctrl=focus_auto=0
    v4l2-ctl -d /dev/video1 --set-ctrl=focus_absolute=0
    v4l2-ctl -d /dev/video1 --set-ctrl=contrast=255
    v4l2-ctl -d /dev/video1 --set-ctrl=sharpness=255
    
    This can also be done with v4l2ucp for the GUI lovers
*/

//this program takes a picture of an artag at 100cm away that is 20cm wide and returns the focal length for cm
int main()
{   
    // Set the necessary variables for aruco
    std::vector<std::vector<cv::Point2f>> corners, rejects;
    std::vector<int> MarkerIDs;
    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    
    cv::FileStorage fs("../urcDict.yml", cv::FileStorage::READ);
    int markerSize, maxCorrBits;
    cv::Mat bits;
    fs["MarkerSize"] >> markerSize;
    fs["MaxCorrectionBits"] >> maxCorrBits;
    fs["ByteList"] >> bits;
    fs.release();
    cv::aruco::Dictionary urcDict = cv::aruco::Dictionary(bits, markerSize, maxCorrBits);
    
    
    cv::VideoCapture cap("/dev/video1"); 
    cap.set(cv::CAP_PROP_FRAME_WIDTH,1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    
    cv::Mat image;
    
    while(true)
    {
        cap >> image;
        parameters->markerBorderBits = 2;
        cv::aruco::detectMarkers(image, &urcDict, corners, MarkerIDs, parameters, rejects);
        
        double widthOfTag = 0;
        if(MarkerIDs.size() > 0)
            widthOfTag = corners[0][1].x - corners[0][0].x;
        else
            std::cout << "nothing found" << std::endl;
            
        std::cout << "Focal Length: " << ((widthOfTag * 100.0) / 20.0) << std::endl;
        cv::imshow("win", image);
        cv::waitKey(100);
    }
}
