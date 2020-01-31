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
        
        ARTracker(std::string mainFile, std::string secondaryFile); //give the video input source
        int findAR(int id); //returns 0 if nothing found, 1 for left, 2 for middle, and 3 for right if a camera finds the tag
        int findARTags(int id1, int id2); //returns camera that finds the tag if its found
        
        bool trackAR(int id);//just uses one camera to find the tag. More efficient
        int trackARTags(int id1, int id2); 
        
        //cv::Mat leftFrame, middleFrame, rightFrame; //the camera frames. Can be used for debugging
        cv::Mat frame, mFrame; //for use when we're just using one camera
        
        
    private:
        cv::VideoCapture mainCap, secondaryCap; 
        aruco::MarkerDetector MDetector; 
        std::vector<aruco::Marker> Markers;
        bool arFound(int id, cv::Mat image);
        
        int widthOfTag = 0;
        int centerXTag = 0;
        float degreesPerPixel = 82.1/640.0; // fov / horizontal resolution. Noah gave me this
        float focalLength = 1380; //611 worked for 640x480, sFor cm. Found using finalFinalLength.cpp
};
