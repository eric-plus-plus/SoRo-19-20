#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "Error, not enough args. Please give the vidoe file to be read from and the directory to be written to" << std::endl;
        exit(-1);
    }

    std::string videoFile(argv[1]);
    cv::VideoCapture cam(videoFile);
    if(!cam.isOpened())
    {
        std::cout << "Error, camera not connected" << std::endl;
        exit(-2);
    }

    cv::Mat im;
    std::string fileName;
    for(int i = 0; i < 150; i++) //writes 10 images at a time
    {
        if(!cam.read(im))
        {
            std::cout << "A camera is broken" << std::endl;
            exit(-3);
        }

        if(i > 100) //gives 50 frames to ensure camera is in position
        {
            auto timeStamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            fileName = std::string(argv[2]) + "/" + std::to_string(timeStamp) + ".jpg";
            cv::imwrite(fileName, im);
        }

        cv::imshow("image", im);
        cv::waitKey(100);
    }
}
