#include <ARTracker.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    ARTracker* tracker = new ARTracker("/dev/video0");
    cv::namedWindow("win");
    while(true)
    {
        cout << tracker->findAR(0) << endl;
        cout << "distance: " << tracker->distanceToAR << endl;
        cout << "angle: " << tracker->angleToAR << endl;
        cv::imshow("win", tracker->frame);
        cv::waitKey(100);
    }
}
