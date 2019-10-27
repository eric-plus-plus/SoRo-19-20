#include <vector>
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <aruco/markerdetector.h>
#include <opencv2/flann.hpp>
#include <opencv2/aruco.hpp>
#include <unistd.h>

class ARTracker
{
    public:
        float angleToAR = 0; //to the one post for findAR, to the center for findARPosts
        float distanceToAR = 0; //see above. Should be in centimeters
        
        ARTracker(std::string videoSource); //give the video input source
        bool findAR(int id); //true if found and updates the angleToAR and distanceToAR vars
        int findARTags(int id1, int id2); //returns number of AR Tags that it sees
        
        cv::Mat frame;
        
    private:
        cv::VideoCapture cap; 
        aruco::MarkerDetector MDetector; 
        std::vector<aruco::Marker> Markers; //to get the id use Markers[i].id;
        
        int widthOfTag = 0;
        int centerXTag = 0;
        float degreesPerPixel = 82.1/640.0; // fov / horizontal resolution
};

ARTracker::ARTracker(std::string videoSource) : cap(videoSource)
{
    if(!cap.isOpened())
    {
        std::cout<< "Unable to open video file: " << videoSource << std::endl;
        exit(-1);
    }
    
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640); //resolution set at 640x480
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    
    cv::namedWindow("win");
    
    MDetector.setDictionary("../urc.dict");
}

bool ARTracker::findAR(int id)
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    
    if(Markers.size() == 0 || Markers[0].id != id) 
    {
        distanceToAR=0;
        angleToAR=0;
        return false; //correct ar tag not found
    }
    else
    {
        //distance = (WIDTH / 2) / tan((pixelWidthOfTag / 2) * degreesPerPixel). Makes assumption of right triangle which I guess doesn't matter
        widthOfTag = Markers[0][1].x - Markers[0][0].x;
        distanceToAR = (20 / 2) / tan(((widthOfTag / 2) * degreesPerPixel) * (3.1415 / 180)); //ar tag's width is 20cm
        
        centerXTag = (Markers[0][1].x + Markers[0][0].x) / 2;
        angleToAR = degreesPerPixel * (centerXTag - 320); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return true;
    }
}

int ARTracker::findARTags(int id1, int id2)
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    if(Markers.size() == 0 || Markers.size() == 1) return Markers.size(); //We may want this to handle finding one post differently. TODO: check ids
    else
    {
        //NOTE: Distance does not matter if we're trying to drive between the posts so I ignored it here
        
        centerXTag = (Markers[0][1].x + Markers[0][0].x + Markers[1][1].x + Markers[1][0].x) / 4; //takes the average of all four x edges of the markers. Could probably cut this to two
        angleToAR = degreesPerPixel * (centerXTag - 320); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return Markers.size();
    }
}
