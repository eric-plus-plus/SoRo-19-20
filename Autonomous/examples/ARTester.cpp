#include <libs/ARTracker.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    ARTracker* tracker = new ARTracker("/dev/video0");
    cv::namedWindow("win"); //This will break the code if run over SSH
    while(true)
    {
        cout << tracker->findAR(5) << endl;
        cout << "distance: " << tracker->distanceToAR << endl;
        cout << "angle: " << tracker->angleToAR << endl;
        cv::imshow("win", tracker->frame); //This will break the code if run over SSH
        cv::waitKey(100);
    }
}
