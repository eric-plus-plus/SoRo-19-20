#include "ARTracker.h"

ARTracker::ARTracker(std::string mainFile, std::string leftFile, std::string rightFile) : mainCap(mainFile), leftCap(leftFile), rightCap(rightFile)
{
    if(!mainCap.isOpened())
    {
        std::cout<< "Unable to open main video file" << std::endl;
        exit(-1);
    }
    
    if(!leftCap.isOpened())
    {
        std::cout<< "Unable to open left video file"<< std::endl;
        exit(-1);
    }
    
    if(!rightCap.isOpened())
    {
        std::cout<< "Unable to open right video file"<< std::endl;
        exit(-1);
    }
    
    mainCap.set(cv::CAP_PROP_FRAME_WIDTH,1920);
    mainCap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    leftCap.set(cv::CAP_PROP_FRAME_WIDTH,1920);
    leftCap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    rightCap.set(cv::CAP_PROP_FRAME_WIDTH,1920);
    rightCap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    mainCap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    leftCap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    rightCap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    mainCap.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    leftCap.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    rightCap.set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc('M', 'J', 'P', 'G') );
    //std::cout << "got here" << std::endl;
    
    MDetector.setDictionary("../urc.dict");
}

bool ARTracker::arFound(int id, cv::Mat image)
{
    cv::cvtColor(image, image, cv::COLOR_RGB2GRAY); //converts to grayscale
    //tries converting to b&w using different different cutoffs to find the perfect one for the ar tag
    for(int i = 40; i <= 220; i+=60)
    {
        Markers = MDetector.detect(image > i);
        if(Markers.size() > 0)
        {
            //mFrame = image > i; //purely for debug
            break;
        }
        else if(i == 220)
        {
            distanceToAR = -1;
            angleToAR = 0;
            return false;
        }
    }
    int index = -1;
    for(int i = 0; i < Markers.size(); i++) //this just checks to make sure that it found the right tag
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

int ARTracker::findAR(int id)
{
    //middle camera checker
    mainCap >> frame;
    if(arFound(id, frame)) return 1;
    
    //left camera checker
    leftCap >> frame;
    if(arFound(id, frame)) return 2;
    
    //right camera checker
    rightCap >> frame;
    if(arFound(id, frame)) return 3;
     
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    return 0;
}

bool ARTracker::trackAR(int id)
{
    //cv::Mat image;
    mainCap >> frame;
    if(arFound(id, frame)) return true;
    return false;
}

/*int ARTracker::trackARTags(int id1, int id2)
{
    middleCap >> frame;
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
}*/
