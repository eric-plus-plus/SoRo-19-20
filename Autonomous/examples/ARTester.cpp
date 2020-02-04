#include <libs/ARTracker.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{    
    if(argc != 3)
        std::cout << "Arguments: main_camera, left_camera, right_camera" << std::endl;    
    cout << argv[1] << "\n" << argv[2] << "\n" << argv[3] << std::endl;
    ARTracker* tracker = new ARTracker(argv[1], argv[2], argv[3]);
    //cv::namedWindow("win"); //This will break the code if run over SSH
    while(true)
    {
        cout << tracker->findAR(0) << endl;
        cout << "distance: " << tracker->distanceToAR << endl;
        cout << "angle: " << tracker->angleToAR << endl;
        //cv::imshow("win", tracker->mFrame); //This will break the code if run over SSH
        cv::waitKey(100);
    }
}
