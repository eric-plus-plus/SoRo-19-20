#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/flann.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>

class ARTracker
{
    public:
        float angleToAR = 0; //to the single post for findAR, to the center for findARPosts
        float distanceToAR = -1; //distance to a single post for findAR or to the center for findARPosts. Should be in centimeters
        
        ARTracker(char* cameras[], std::string format); //give the video input source
        bool findAR(int id); //false if nothing found, true else. Updates angleToAR and distanceToAR
        bool findARs(int id1, int id2); //same as above except looking for the gate instead of a single post
        
        bool trackAR(int id);//findAR but just uses one camera to find the tag. More efficient and all that is needed for the tracking phase
        bool trackARs(int id1, int id2); //same as findARs except looking for a gate instead of a single post
        
        cv::Mat frame, mFrame; //mFrame isn't really used but good to have for debug
        
        cv::VideoWriter videoWriter; //gives way of writting the video to disk
        
        
    private:
        std::vector<cv::VideoCapture*> caps; 
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::Dictionary urcDict; 
        std::vector<int> MarkerIDs;
        bool arFound(int id, cv::Mat image, bool writeToFile); //returns true when tag w/ correct id is found
        int countValidARs(int id1, int id2, cv::Mat image, bool writeToFile); //returns number (0, 1, or 2) of tags found w/ correct ids
        bool config();
        
        int distanceToAR1 = 0;
        int distanceToAR2 = 0;
        int widthOfTag = 0;
        int widthOfTag1 = 0;
        int widthOfTag2 = 0;
        int centerXTag = 0;
        int frameWidth = 1920, frameHeight = 1080; //Width and Height are only for the writer.
        int knownTagWidth; //known physical width of the AR tag. Gotten from config
        float degreesPerPixel; // fov / horizontal resolution. Gotten from config
        float focalLength; //611 worked for 640x480, for cm. Gotten from the config which is found using finalFinalLength.cpp
};
