#include <vector>
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <aruco/markerdetector.h>
#include <opencv2/flann.hpp>
#include <unistd.h>

class ARTracker
{
    public:
        float angleToAR = 0; //to the one post for findAR, to the center for findARPosts
        float distanceToAR = -1; //see above. Should be in centimeters
        
        ARTracker(std::string videoSource); //give the video input source
        bool findAR(int id); //true if found and updates the angleToAR and distanceToAR vars
        int findARTags(int id1, int id2); //returns number of correct AR Tags that it sees
        
        cv::Mat frame; //the camera frame. Can be used for debugging
        
    private:
        cv::VideoCapture cap; 
        aruco::MarkerDetector MDetector; 
        std::vector<aruco::Marker> Markers;
        
        int widthOfTag = 0;
        int centerXTag = 0;
        float degreesPerPixel = 82.1/640.0; // fov / horizontal resolution. Noah gave me this
        float focalLength = 611; //For cm. Found using finalFinalLength.cpp
        float differenceParameter = 0.0;
        float distanceParameter = 1.0;
};
