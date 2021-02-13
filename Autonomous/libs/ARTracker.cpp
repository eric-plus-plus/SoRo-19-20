#include "ARTracker.h"

//Reads the file and sets the variables need in the class
bool ARTracker::config() 
{
    std::ifstream file;
    std::string line, info;
	std::vector<std::string> lines;
    file.open("../config.txt");
    if(!file.is_open())
        return false;
    while(getline(file, line)) 
    {
        //info += line;
        lines.push_back(line);
    }
	for(int i = 0; i < lines.size(); ++i) 
	{
		if(lines[i].find("DEGREES_PER_PIXEL=") != std::string::npos) 
			degreesPerPixel = std::stod(lines[i].substr(lines[i].find("DEGREES_PER_PIXEL=") + 18));
		if(lines[i].find("FOCAL_LENGTH=") != std::string::npos) 
			focalLength = std::stod(lines[i].substr(lines[i].find("FOCAL_LENGTH=") + 13));
		if(lines[i].find("KNOWN_TAG_WIDTH=") != std::string::npos) 
			knownTagWidth = std::stoi(lines[i].substr(lines[i].find("KNOWN_TAG_WIDTH=") + 16));
	}
	return true;
}  

//The writer size is hard coded here. You've been warned
ARTracker::ARTracker(char* cameras[], std::string format) : videoWriter("autonomous.avi", cv::VideoWriter::fourcc(format[0],format[1],format[2],format[3]), 5, cv::Size(1920,1080), false)
{
    if(!config())
        std::cout << "Error opening file" << std::endl;
    
    // Read in the dictionary from the file
    cv::FileStorage fs("../urcDict.yml", cv::FileStorage::READ);
    int markerSize, maxCorrBits;
    cv::Mat bits;
    fs["MarkerSize"] >> markerSize;
    fs["MaxCorrectionBits"] >> maxCorrBits;
    fs["ByteList"] >> bits;
    fs.release();
    urcDict = cv::aruco::Dictionary(bits, markerSize, maxCorrBits);
    dictPtr = &urcDict; //put the dict from the file into the opencv Ptr

    for(int i = 0; true; i++) //initializes the cameras
    {
        if(cameras[i] == NULL)
            break;
        caps.push_back(new cv::VideoCapture(cameras[i]));//, cv::CAP_V4L2));
        if(!caps[i]->isOpened())
        {
            std::cout << "Camera " << cameras[i] << " did not open!" << std::endl;
            exit(-1);
        }
        caps[i]->set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
        caps[i]->set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);
        caps[i]->set(cv::CAP_PROP_BUFFERSIZE, 1); //greatly speeds up the program but the writer is a bit wack because of this
        caps[i]->set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc(format[0], format[1], format[2], format[3]) );
    }
}

bool ARTracker::arFound(int id, cv::Mat image, bool writeToFile)
{
    cv::cvtColor(image, image, cv::COLOR_RGB2GRAY); //converts to grayscale
    
    int index = -1; 
    //tries converting to b&w using different different cutoffs to find the perfect one for the current lighting
    for(int i = 40; i <= 220; i+=60)
    {
        parameters->markerBorderBits = 2; 
        cv::aruco::detectMarkers((image > i), dictPtr, corners, MarkerIDs, parameters, rejects); //detects all of the tags in the current b&w cutoff
 
 	if(MarkerIDs.size() > 0)
        {
            index = -1;
            for(int i = 0; i < MarkerIDs.size(); i++) //this just checks to make sure that it found the right tag. Probably should move this into the b&w block
            {
                if(MarkerIDs[i] == id)
                {
                    index = i;
                    break; 
                }   
            }
            
            if(index == -1) 
            {
                std::cout << "Found a tag but was not the correct one" << std::endl;
            } 
            else
            {
		        std::cout << "Found the correct tag!" << std::endl;
                if(writeToFile)
                {
                    mFrame = image > i; //purely for debug
                    videoWriter.write(mFrame); //purely for debug
                }    
                break;
            }
        }
        else if(i == 220) //did not find any AR tags with any b&w cutoff
        {
            if(writeToFile)
                videoWriter.write(image);
            distanceToAR = -1;
            angleToAR = 0;
            return false;
        }
    }
        
    widthOfTag = corners[index][1].x - corners[index][0].x;
    //distanceToAR = (knownWidthOfTag(20cm) * focalLengthOfCamera) / pixelWidthOfTag
    distanceToAR = (knownTagWidth * focalLength) / widthOfTag;
    
    centerXTag = (corners[index][1].x + corners[index][0].x) / 2;
    angleToAR = degreesPerPixel * (centerXTag - 960); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
    
    return true;
}

int ARTracker::countValidARs(int id1, int id2, cv::Mat image, bool writeToFile)
{
    cv::cvtColor(image, image, cv::COLOR_RGB2GRAY); //converts to grayscale
    
    //tries converting to b&w using different different cutoffs to find the perfect one for the ar tag
    for(int i = 40; i <= 220; i+=60)
    {
        parameters->markerBorderBits = 2; 
        cv::aruco::detectMarkers((image > i), dictPtr, corners, MarkerIDs, parameters, rejects);
        if(MarkerIDs.size() > 0)
        {
            if(MarkerIDs.size() == 1)
            {
                std::cout << "Just found one post" << std::endl;
            }

            else
            {
                if(writeToFile)
                {
                    mFrame = image > i; //purely for debug
                    videoWriter.write(mFrame); //purely for debug
                }
                break;
            }
        }
        
        if(i == 220) //did not ever find two ars. TODO add something for if it finds one tag
        {
            if(writeToFile)
                videoWriter.write(image);
            distanceToAR = -1;
            angleToAR = 0;
            return 0;
        }
    }

    int index1 = -1, index2 = -1;
    for(int i = 0; i < MarkerIDs.size(); i++) //this just checks to make sure that it found the right tags
    {
        if(MarkerIDs[i] == id1 || MarkerIDs[i] == id2)
        {
            if(MarkerIDs[i] == id1)
                index1 = i;
            else
                index2=i;
            
            if(index1 != -1 && index2 != -1)
                break;
        }   
    }
    if(index1 == -1 || index2 == -1) 
    {
        distanceToAR=-1;
        angleToAR=0;
        std::cout << "index1: " << index1 << "\nindex2: " << index2 << std::endl;
        if(index1 != -1 || index2 != -1)
        {
            return 1;
        }
        return 0; //no correct ar tags found
    }
    else
    {
        widthOfTag1 = corners[index1][1].x - corners[index1][0].x;
        widthOfTag2 = corners[index2][1].x - corners[index2][0].x;

        //distanceToAR = (knownWidthOfTag(20cm) * focalLengthOfCamera) / pixelWidthOfTag
        distanceToAR1 = (knownTagWidth * focalLength) / widthOfTag1;
        distanceToAR2 = (knownTagWidth * focalLength) / widthOfTag2;
        std::cout << "1: " << distanceToAR1 << "\n2: " << distanceToAR2 << std::endl;
        std::cout << "focal: " << focalLength << "\nwidth: " << widthOfTag << std::endl;
        distanceToAR = (distanceToAR1 + distanceToAR2) / 2;
        std::cout << distanceToAR << std::endl;
        
        centerXTag = (corners[index1][1].x + corners[index2][0].x) / 2;
        angleToAR = degreesPerPixel * (centerXTag - 960); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return 2;
    }
}


bool ARTracker::findAR(int id)
{    
    for(int i = 0; i < caps.size(); i++)
    {
        *caps[i] >> frame;
        if(arFound(id, frame, false)) return true;
    }
    return false;
}

bool ARTracker::findARs(int id1, int id2)
{
    for(int i = 0; i < caps.size(); i++)
    {
        *caps[i] >> frame;
        if(countValidARs(id1, id2, frame, false)) return true;
    }
    return false;

}

bool ARTracker::trackAR(int id)
{
    //cv::Mat image;
    *caps[0] >> frame;
    if(arFound(id, frame, false)) return true;
    return false;
}

bool ARTracker::trackARs(int id1, int id2)
{
    //cv::Mat image;
    *caps[0] >> frame;
    if(countValidARs(id1, id2, frame, false)) return true;
    return false;
}
