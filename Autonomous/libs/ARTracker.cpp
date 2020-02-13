#include "ARTracker.h"

ARTracker::ARTracker(char* cameras[], std::string format) : videoWriter("autonomous.avi", cv::VideoWriter::fourcc('M','J','P','G'), 10, cv::Size(1920,1080), false) //for the SAR and debug I guess
{
    for(int i = 0; true; i++) //initializes the cameras
    {
        if(cameras[i] == NULL)
            break;
        caps.push_back(new cv::VideoCapture(cameras[i]));
        if(!caps[i]->isOpened())
        {
            std::cout << "Camera " << cameras[i] << " did not open!" << std::endl;
            exit(-1); //TODO make it so it doesn't quit here
        }
        caps[i]->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        caps[i]->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        caps[i]->set(cv::CAP_PROP_BUFFERSIZE, 1);
        caps[i]->set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc(format[0], format[1], format[2], format[3]) );
    }
    //std::cout << "got here" << std::endl;
    
    MDetector.setDictionary("../urc.dict");
}

bool ARTracker::arFound(int id, cv::Mat image, bool writeToFile)
{
    cv::cvtColor(image, image, cv::COLOR_RGB2GRAY); //converts to grayscale
    //tries converting to b&w using different different cutoffs to find the perfect one for the ar tag
    for(int i = 40; i <= 220; i+=60)
    {
        Markers = MDetector.detect(image > i);
        if(Markers.size() > 0)
        {
            if(writeToFile)
                videoWriter.write(image > i); //purely for debug
            //mFrame = image > i; //purely for debug
            break;
        }
        else if(i == 220)
        {
            if(writeToFile)
                videoWriter.write(image);
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

bool ARTracker::findAR(int id)
{    
    for(int i = 0; i < caps.size(); i++)
    {
        *caps[i] >> frame;
        if(arFound(id, frame, false)) return true;
    }
    //std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    return false;
}

bool ARTracker::trackAR(int id)
{
    //cv::Mat image;
    *caps[0] >> frame;
    if(arFound(id, frame, true)) return true;
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
