#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/flann.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>
#include <chrono>

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "Error, not enough args. Please give the vidoe file to be read from and the directory to be written to" << std::endl;
        exit(-1);
    }

    int id = 10;

    std::string videoFile(argv[1]);
    cv::VideoCapture cam(videoFile);
    if(!cam.isOpened())
    {
        std::cout << "Error, camera not connected" << std::endl;
        exit(-2);
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    cam.set(cv::CAP_PROP_BUFFERSIZE, 1); //greatly speeds up the program but the writer is a bit wack because of this
   cam.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );

    std::vector<std::vector<cv::Point2f>> corners, rejects; // rejects will likely be unused
    cv::Ptr<cv::aruco::Dictionary> dictPtr; //detectMarkers() needs an opencv Ptr for the dict
    cv::aruco::Dictionary urcDict; //this is a semi-temp var, the dict is read from the file and then put into dictPtr
    std::vector<int> MarkerIDs;
    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();

    // Read in the dictionary from the file
    cv::FileStorage fs("../../urcDict.yml", cv::FileStorage::READ);
    int markerSize, maxCorrBits;
    cv::Mat bits;
    fs["MarkerSize"] >> markerSize;
    fs["MaxCorrectionBits"] >> maxCorrBits;
    fs["ByteList"] >> bits;
    fs.release();
    urcDict = cv::aruco::Dictionary(bits, markerSize, maxCorrBits);
    dictPtr = &urcDict; //put the dict from the file into the opencv Ptr

    cv::Mat inIm;
    cv::Mat arIm;
    std::string imageFileName, labelFileName;
    int top = -1, bottom = -1, right = -1, left = -1;
    for(int i = 0; i < 150; i++) //writes 10 images at a time
    {
        if(!cam.read(inIm))
        {
            std::cout << "A camera is broken" << std::endl;
            exit(-3);
        }
        
        cv::cvtColor(inIm, arIm, cv::COLOR_RGB2GRAY);
        parameters->markerBorderBits = 1;
        int index = -1; 
        //tries converting to b&w using different different cutoffs to find the perfect one for the current lighting
        for(int i = 40; i <= 220; i+=60)
        {        
            cv::aruco::detectMarkers((arIm > i), dictPtr, corners, MarkerIDs, parameters, rejects); //detects all of the tags in the current b&w cutoff

            if(MarkerIDs.size() > 0)
            {
                index = -1;
                for(int i = 0; i < MarkerIDs.size(); i++) //this just checks to make sure that it found the right tag. Probably should move this into the b&w block
                {
                   // std::cout << corners[i][1].x - corners[i][0].x << "\n\n";
                    if(MarkerIDs[i] == id)
                    {
                        index = i;
                        break; 
                    }   
                }
                
                if(index != -1)
                {
                    std::cout << "Found the correct tag!" << std::endl;   
                    break;
                }
	        else
                   std::cout << "Found a tag but was not the correct one" << std::endl;
            }
            if(i == 220) //did not find any AR tags with any b&w cutoff
            {
                break;
            }
        }
        
        if(index > -1)
        {
            right = corners[index][1].x;
            left = corners[index][0].x;
            top = corners[index][1].y;
            bottom = corners[index][3].y;
        }
        
        //std::cout << right << ", " << left << ", " << top << ", " << bottom << std::endl;
        //cv::aruco::drawDetectedMarkers(arIm, corners, MarkerIDs);
        
        for(int i = 0; i < arIm.rows; i++)
        {
            for(int j = 0; j < arIm.cols; j++)
            {
                if(j > left && j < right && i > top && i < bottom)
                    arIm.at<uchar>(i,j) = 255;
                else
                    arIm.at<uchar>(i,j) = 0;
            }
        }
        
        
        if(i > 5) //gives 50 frames to ensure camera is in position
        {
            if(index != -1)
            {
                auto timeStamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                imageFileName = std::string(argv[2]) + "/" + std::to_string(timeStamp) + ".jpg";
                labelFileName = std::string(argv[3]) + "/" + std::to_string(timeStamp) + ".jpg";
                cv::imwrite(imageFileName, inIm);
                cv::imwrite(labelFileName, arIm);
            }
        }
        
        cv::imshow("image", inIm);
        cv::waitKey(100);
    }
}
