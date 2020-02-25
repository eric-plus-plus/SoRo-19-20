#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <aruco/markerdetector.h>
#include <opencv2/flann.hpp>
#include <unistd.h>

class ARTracker
{
    public:
        float angleToAR = 0; //to the single post for findAR, to the center for findARPosts
        float distanceToAR = -1; //see above. Should be in centimeters
        
        ARTracker(char* cameras[], std::string format); //give the video input source
        bool findAR(int id); //false if nothing found, true else
        bool findARs(int id1, int id2);
        int findARTags(int id1, int id2); //returns camera that finds the tag if its found
        
        bool trackAR(int id);//just uses one camera to find the tag. More efficient
        bool trackARs(int id1, int id2);
        int trackARTags(int id1, int id2); 
        
        cv::Mat frame, mFrame; //for use when we're just using one camera. mFrame isn't really used but good to have for debug
        
        cv::VideoWriter videoWriter;
        
        
    private:
        std::vector<cv::VideoCapture*> caps; 
        aruco::MarkerDetector MDetector; 
        std::vector<aruco::Marker> Markers;
        bool arFound(int id, cv::Mat image, bool writeToFile); //returns true when tag w/ correct id is found
        int countValidARs(int id1, int id2, cv::Mat image, bool writeToFile); //returns number (0, 1, or 2) of tags found w/ correct ids
		    bool config();
        
        int widthOfTag = 0;
        int centerXTag = 0;
        float degreesPerPixel; // fov / horizontal resolution
        float focalLength; //611 worked for 640x480, for cm. Found using finalFinalLength.cpp
};
