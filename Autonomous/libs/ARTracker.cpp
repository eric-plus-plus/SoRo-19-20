#include "ARTracker.h"

//Reads the file and sets the variables need in the class
bool ARTracker::config() 
{
    std::ifstream file;
    std::string line, info;
	std::size_t found;
    file.open("config.txt");
    if(!file.is_open())
		return false;
    while(getline(file, line)) 
    {
		info += line;
	}
	//The numbers there will correctly parse the proper sized substring
	degreesPerPixel = std::stod(info.substr(info.find("DEGREES_PER_PIXEL=") + 18, info.find("FOCAL_LENGTH=" - 18)));
	focalLength = std::stod(info.substr(info.find("FOCAL_LENGTH=") + 13, 4));
	return true;
}  

ARTracker::ARTracker(char* cameras[], std::string format) : videoWriter("autonomous.avi", cv::VideoWriter::fourcc(format[0],format[1],format[2],format[3]), 5, cv::Size(1920,1080), false)
{
	if(!config())
		std::cout << "Error opening file" << std::endl;
    for(int i = 0; true; i++) //initializes the cameras
    {
        if(cameras[i] == NULL)
            break;
        caps.push_back(new cv::VideoCapture(cameras[i]));
        if(!caps[i]->isOpened())
        {
            std::cout << "Camera " << cameras[i] << " did not open!" << std::endl;
            exit(-1);
        }
        caps[i]->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        caps[i]->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        caps[i]->set(cv::CAP_PROP_BUFFERSIZE, 1);
        caps[i]->set(cv::CAP_PROP_FOURCC ,cv::VideoWriter::fourcc(format[0], format[1], format[2], format[3]) );
    }
    
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
            {
		        mFrame = image > i; //purely for debug
                videoWriter.write(mFrame); //purely for debug
            }    
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
    return false;
}

bool ARTracker::trackAR(int id)
{
    //cv::Mat image;
    *caps[0] >> frame;
    if(arFound(id, frame, false)) return true;
    return false;
}
