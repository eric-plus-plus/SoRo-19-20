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
        float distanceToAR = -1; //see above. Should be in centimeters
        
        ARTracker(std::string videoSource); //give the video input source
        bool findAR(int id); //true if found and updates the angleToAR and distanceToAR vars
        int findARTags(int id1, int id2); //returns number of correct AR Tags that it sees
        
        cv::Mat frame; //the camera frame. Mostly for debugging
        
    private:
        cv::VideoCapture cap; 
        aruco::MarkerDetector MDetector; 
        std::vector<aruco::Marker> Markers; //to get the id use Markers[i].id;
        
        int widthOfTag = 0;
        int centerXTag = 0;
        float degreesPerPixel = 82.1/640.0; // fov / horizontal resolution. Noah gave me this
};

ARTracker::ARTracker(std::string videoSource) : cap(videoSource)
{
    if(!cap.isOpened())
    {
        std::cout<< "Unable to open video file: " << videoSource << std::endl;
        exit(-1);
    }
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH,640); //resolution set at 640x480
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    cv::namedWindow("win"); //creates the window. Use this for debug
    
    MDetector.setDictionary("../urc.dict");
}

bool ARTracker::findAR(int id)
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    
    int index = -1;
    for(int i = 0; i < Markers.size(); i++)
    {
        if(Markers[i].id == id)
        {
            index = i;
            break;  
        }   
    }
    if(index == -1) 
    {
        distanceToAR=-1;
        angleToAR=0;
        return false; //correct ar tag not found
    }
    else
    {
        //distance = (WIDTH / 2) / tan((pixelWidthOfTag / 2) * degreesPerPixel). Makes assumption of right triangle which I guess doesn't matter
        widthOfTag = Markers[index][1].x - Markers[index][0].x;
        distanceToAR = (20 / 2) / tan(((widthOfTag / 2) * degreesPerPixel) * (3.1415 / 180)); //ar tag's width is 20cm
        
        centerXTag = (Markers[index][1].x + Markers[index][0].x) / 2;
        angleToAR = degreesPerPixel * (centerXTag - 320); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return true;
    }
}

int ARTracker::findARTags(int id1, int id2)
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    
    int id1Index = -1;
    int id2Index = -1;
    
    //finds how many of the correct ids were found
    int idsFound = 0;
    for(int i = 0; i < Markers.size(); i++)
    {
        if(Markers[i].id == id1 && id1Index == -1)
        {
            idsFound++; 
            id1Index=i;
        }
        else if(Markers[i].id == id2 && id2Index == -1)
        {
            idsFound++;
            id2Index=i;
        }
    }
    if(idsFound < 2) // did not find the two correct tags. Later may want to return which ones weren't found for the searching algorithim but IDK for sure.
    {
        angleToAR=0;
        return idsFound;
    }
    else 
    {
        //NOTE: Distance does not matter if we're trying to drive between the posts so I ignored it here
        
        centerXTag = (Markers[id1Index][1].x + Markers[id1Index][0].x + Markers[id2Index][1].x + Markers[id2Index][0].x) / 4; //takes the average of all four x edges of the markers. Could probably cut this to two
        angleToAR = degreesPerPixel * (centerXTag - 320); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return Markers.size();
    }
}
