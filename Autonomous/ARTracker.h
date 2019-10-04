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
        float angleToAR; //to the one post for findAR, to the center for findARPosts
        float distanceToAR; //see above
        
        bool betweenPosts; //no clue how this is happening
        
        ARTracker(std::string videoSource); //give the video input source
        bool findAR(); //true if found and updates the angleToAR and distanceToAR vars
        int findARPosts(); //returns number of AR Tags that it sees
        
    private:
        cv::VideoCapture cap; 
        aruco::MarkerDetector MDetector;
        std::vector<aruco::Marker> Markers;
        cv::Mat frame;
        int post1area;
        int post2area;
        int post1distance;
        int post2distance;
        int centerPost;
};

ARTracker::ARTracker(std::string videoSource) : cap(videoSource)
{
    if(!cap.isOpened())
    {
        std::cout<< "Unable to open video file: " << videoSource << std::endl;
        exit(-1);
    }
    
    //set dictionary here if needed
}

bool ARTracker::findAR()
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    if(Markers.size() == 0) return false; //no aruco markers found
    else
    {
        post1area = (Markers[0][1].x - Markers[0][0].x) * (Markers[0][3].y - Markers[0][0].y);
        //TODO: convert area to distance
        
        centerPost = (Markers[0][1].x + Markers[0][0].x) / 2;
        //TODO: convert the above to an angle
        
        return true;
    }
}

bool ARTracker::findARPosts()
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    if(Markers.size() == 0 || 1) return Markers.size(); //We may want this to handle finding one post differently
    else
    {
        post1area = (Markers[0][1].x - Markers[0][0].x) * (Markers[0][3].y - Markers[0][0].y);
        //TODO: convert area to distance
        post2area = (Markers[1][1].x - Markers[1][0].x) * (Markers[1][3].y - Markers[1][0].y);
        //TODO: convert area to distance
        
        centerPost = (Markers[0][1].x + Markers[0][0].x + Markers[1][1].x + Markers[1][0].x) / 4; //takes the average of all four x edges of the markers. Could probably cut this to two
        //TODO: convert the above to an angle to the center of the posts.
    }
}
