#include "libs/ARTracker.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{  
    int returnVal; 
    std::cout << argv[0] << "\n" << argv[1] << std::endl;
    ARTracker tracker(argv + 1, "MJPG");
    //cv::namedWindow("win"); //This will break the code if run over SSH
    while(true)
    {
        returnVal = tracker.findAR(10);
        cout << "Return value: " << returnVal << endl;
        cout << "distance: " << tracker.distanceToAR << endl;
        cout << "angle: " << tracker.angleToAR << endl;
        //cv::imshow("win", tracker->mFrame); //This will break the code if run over SSH
        cv::waitKey(100);
    }
}
