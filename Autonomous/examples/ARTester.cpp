#include <libs/ARTracker.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{    
    if(argc != 3)
        std::cout << "Arguments: left_camera_file, middle_camera_file, right_camera_file" << std::endl;    
    ARTracker* tracker = new ARTracker(argv[0], argv[1], argv[2]);
    cv::namedWindow("win"); //This will break the code if run over SSH
    while(true)
    {
        cout << tracker->trackAR(5) << endl;
        cout << "distance: " << tracker->distanceToAR << endl;
        cout << "angle: " << tracker->angleToAR << endl;
        cv::imshow("win", tracker->frame); //This will break the code if run over SSH
        cv::waitKey(100);
    }
}
