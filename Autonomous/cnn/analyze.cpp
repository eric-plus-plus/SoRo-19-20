#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>

int main()
{
    int width = 1920, height= 1080;
    cv::Mat im = cv::imread("../test.jpg");
    cv::Mat realIm = im(cv::Rect(0,0,width,height));
    cv::Mat predIm = im(cv::Rect(0,height,width,height));
    cv::cvtColor(predIm, predIm, cv::COLOR_BGR2GRAY);

    // find contours (if always so easy to segment as your image, you could just add the black/rect pixels to a vector)
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(predIm,contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    /// Draw contours and find biggest contour (if there are other contours in the image, we assume the biggest one is the desired rect)
    // drawing here is only for demonstration!
    int biggestContourIdx = -1;
    float biggestContourArea = 0;
    cv::Mat drawing = cv::Mat::zeros( predIm.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        cv::Scalar color = cv::Scalar(0, 100, 0);
        drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );

        float ctArea= cv::contourArea(contours[i]);
        if(ctArea > biggestContourArea)
        {
            bool lowY = false;
            for(int j = 0; j < contours[i].size(); j++)
            {
                if(contours[i][j].y > 50)
                    lowY = true;
            }
            
            if(lowY)
            {
                biggestContourArea = ctArea;
                biggestContourIdx = i;
            }
        }
    }

    // if no contour found
    if(biggestContourIdx < 0)
    {
        std::cout << "no contour found" << std::endl;
        return 1;
    }

    // compute the rotated bounding rect of the biggest contour! (this is the part that does what you want/need)
    cv::RotatedRect boundingBox = cv::minAreaRect(contours[biggestContourIdx]);

    // draw the rotated rect
    cv::Point2f corners[4];
    boundingBox.points(corners);
    cv::line(drawing, corners[0], corners[1], cv::Scalar(255,0,0));
    cv::line(drawing, corners[1], corners[2], cv::Scalar(0,255,0));
    cv::line(drawing, corners[2], corners[3], cv::Scalar(0,0,255));
    cv::line(drawing, corners[3], corners[0], cv::Scalar(255,255,0));
    
    std::cout << corners[0] << std::endl;
    std::cout << corners[1] << std::endl;
    std::cout << corners[2] << std::endl;
    std::cout << corners[3] << std::endl;

    // display
    cv::imshow("input", predIm);
    cv::imshow("drawing", drawing);
    cv::waitKey(0);

}
