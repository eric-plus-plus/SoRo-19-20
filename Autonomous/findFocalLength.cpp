#include <vector>
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <aruco/markerdetector.h>
#include <opencv2/flann.hpp>
#include <opencv2/aruco.hpp>
#include <unistd.h>

//this program takes a picture of an artag at 100cm away that is 20cm wide and returns the focal length for cm
int main()
{   
    cv::Mat iamge;
    aruco::MarkerDetector MDetector; 
    std::vector<aruco::Marker> Markers;
    MDetector.setDictionary("../urc.dict");
    cv::Mat image = cv::imread("/dev/video0");
    Markers = MDetector.detect(image);
    
    int widthOfTag = 0;
    if(Markers.size() > 0)
        widthOfTag = Markers[index][1].x - Markers[index][0].x;
    else
        return -2;
        
    std::cout << "Focal Length: " << ((widthOfTag * 100) / 20) << std::endl;
}
