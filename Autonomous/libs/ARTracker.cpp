#include "ARTracker.h"

ARTracker::ARTracker(std::string videoSource) : cap(videoSource)
{
    if(!cap.isOpened())
    {
        std::cout<< "Unable to open video file: " << videoSource << std::endl;
        exit(-1);
    }
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH,1920); //resolution set at 640x480
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    MDetector.setDictionary("../urc.dict");
}

bool ARTracker::findAR(int id)
{
    //cv::Mat image;
    cap >> frame;
    
    //filters the image
    cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
    //cv::GaussianBlur(image,frame,cv::Size(0, 0),5);
    //cv::addWeighted(image, 2.5, frame, -1.5, 0, frame);
    //frame = (frame > 200);
    //cv::blur( frame, frame, cv::Size(3,3) );
    
    for(int i = 40; i <= 220; i+=60)
    {
        Markers = MDetector.detect(frame > i);
        if(Markers.size() > 0)
        {
            frame = frame > i; //purely for debug
            break;
        }
        if(i == 220)
        {
            std::cout << "no tags found!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            distanceToAR=-1;
            angleToAR=0;
            return false;
        }
    }
    
    int index = -1;
    for(int i = 0; i < Markers.size(); i++)
    {
        if(Markers[i].id == id)
        {
            index = i;
            break;  
        }   
    }
    if(index == -1) 
    {
        distanceToAR=-1;
        angleToAR=0;
        return false; //correct ar tag not found
    }
    else
    {
        widthOfTag = Markers[index][1].x - Markers[index][0].x;
        //distanceToAR = (knownWidthOfTag(20cm) * focalLengthOfCamera) / pixelWidthOfTag
        distanceToAR = (20 * focalLength) / widthOfTag;
        
        centerXTag = (Markers[index][1].x + Markers[index][0].x) / 2;
        angleToAR = degreesPerPixel * (centerXTag - 960); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return true;
    }
}

int ARTracker::findARTags(int id1, int id2)
{
    cap >> frame;
    Markers = MDetector.detect(frame);
    
    int id1Index = -1;
    int id2Index = -1;
    
    //finds how many of the correct ids were found
    int idsFound = 0;
    for(int i = 0; i < Markers.size(); i++)
    {
        if(Markers[i].id == id1 && id1Index == -1)
        {
            idsFound++; 
            id1Index=i;
        }
        else if(Markers[i].id == id2 && id2Index == -1)
        {
            idsFound++;
            id2Index=i;
        }
    }
    if(idsFound < 2) // did not find the two correct tags. Later may want to return which ones weren't found for the searching algorithim but IDK for sure.
    {
        angleToAR=0;
        return idsFound;
    }
    else 
    {
        //NOTE: Distance does not matter if we're trying to drive between the posts so I ignored it here
        
        centerXTag = (Markers[id1Index][1].x + Markers[id1Index][0].x + Markers[id2Index][1].x + Markers[id2Index][0].x) / 4; //takes the average of all four x edges of the markers. Could probably cut this to two
        angleToAR = degreesPerPixel * (centerXTag - 320); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return Markers.size();
    }
}
