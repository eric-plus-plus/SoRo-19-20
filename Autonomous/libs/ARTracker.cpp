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

    // MANUALLY ADD EACH MARKER
    urcDict.markerSize = 5;
    urcDict.maxCorrectionBits = 0;
    
    char bits0[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,1,1,1,1}, {1,1,1,1,1}};
    cv::Mat markerBits0 = cv::Mat(5, 5, CV_8UC1, &bits0);
    cv::Mat markerCompressed0 = cv::aruco::Dictionary::getByteListFromBits(markerBits0);
    urcDict.bytesList.push_back(markerCompressed0);

    char bits1[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {0,0,1,1,0}, {1,1,1,0,1}};
    cv::Mat markerBits1 = cv::Mat(5, 5, CV_8UC1, &bits1);
    cv::Mat markerCompressed1 = cv::aruco::Dictionary::getByteListFromBits(markerBits1);
    urcDict.bytesList.push_back(markerCompressed1);

    char bits2[5][5]  = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,0,1,1,0}, {1,0,1,1,0}};
    cv::Mat markerBits2 = cv::Mat(5, 5, CV_8UC1, &bits2);
    cv::Mat markerCompressed2 = cv::aruco::Dictionary::getByteListFromBits(markerBits2);
    urcDict.bytesList.push_back(markerCompressed2);

    char bits3[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {0,1,1,1,1}, {1,0,1,0,0}};
    cv::Mat markerBits3 = cv::Mat(5, 5, CV_8UC1, &bits3);
    cv::Mat markerCompressed3 = cv::aruco::Dictionary::getByteListFromBits(markerBits3);
    urcDict.bytesList.push_back(markerCompressed3);

    char bits4[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {0,1,1,1,0}, {0,1,1,1,0}};
    cv::Mat markerBits4 = cv::Mat(5, 5, CV_8UC1, &bits4);
    cv::Mat markerCompressed4 = cv::aruco::Dictionary::getByteListFromBits(markerBits4);
    urcDict.bytesList.push_back(markerCompressed4);

    char bits5[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,0,1,1,1}, {0,1,1,0,0}};
    cv::Mat markerBits5 = cv::Mat(5, 5, CV_8UC1, &bits5);
    cv::Mat markerCompressed5 = cv::aruco::Dictionary::getByteListFromBits(markerBits5);
    urcDict.bytesList.push_back(markerCompressed5);

    char bits6[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {0,0,1,1,1}, {0,0,1,1,1}};
    cv::Mat markerBits6 = cv::Mat(5, 5, CV_8UC1, &bits6);
    cv::Mat markerCompressed6 = cv::aruco::Dictionary::getByteListFromBits(markerBits6);
    urcDict.bytesList.push_back(markerCompressed6);

    char bits7[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,1,1,1,0}, {0,0,1,0,1}};
    cv::Mat markerBits7 = cv::Mat(5, 5, CV_8UC1, &bits7);
    cv::Mat markerCompressed7 = cv::aruco::Dictionary::getByteListFromBits(markerBits7);
    urcDict.bytesList.push_back(markerCompressed7);

    char bits8[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {0,0,1,0,1}, {1,1,1,1,0}};
    cv::Mat markerBits8 = cv::Mat(5, 5, CV_8UC1, &bits8);
    cv::Mat markerCompressed8 = cv::aruco::Dictionary::getByteListFromBits(markerBits8);
    urcDict.bytesList.push_back(markerCompressed8);

    char bits9[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,1,1,0,0}, {1,1,1,0,0}};
    cv::Mat markerBits9 = cv::Mat(5, 5, CV_8UC1, &bits9);
    cv::Mat markerCompressed9 = cv::aruco::Dictionary::getByteListFromBits(markerBits9);
    urcDict.bytesList.push_back(markerCompressed9);

    char bits10[5][5] = {{1,1,0,1,1}, {1,1,0,1,1}, {1,0,1,0,1}, {0,1,1,0,0}, {1,0,1,1,1}};
    cv::Mat markerBits10 = cv::Mat(5, 5, CV_8UC1, bits10);
    cv::Mat markerCompressed10 = cv::aruco::Dictionary::getByteListFromBits(markerBits10);
    urcDict.bytesList.push_back(markerCompressed10);

    // PRINT DICTIONARY FOR FURTHER USE
    cv::FileStorage fs("../urcDict.yml", cv::FileStorage::WRITE);
    fs << "MarkerSize" << urcDict.markerSize;
    fs << "MaxCorrectionBits" << urcDict.maxCorrectionBits;
    fs << "ByteList" << urcDict.bytesList;
    fs.release();

    /*// Read in the dictionary from the file
    cv::FileStorage fs("../urcDict.yml", cv::FileStorage::READ);
    int markerSize, maxCorrBits;
    cv::Mat bits;
    fs["MarkerSize"] >> markerSize;
    fs["MaxCorrectionBits"] >> maxCorrBits;
    fs["ByteList"] >> bits;
    fs.release();
    urcDict = cv::aruco::Dictionary(bits, markerSize, maxCorrBits);
    dictPtr = &urcDict; //put the dict from the file into the opencv Ptr*/
    

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
        parameters->minCornerDistanceRate = 0.25;
        parameters->minMarkerPerimeterRate = 0.25;
        //parameters->polygonalApproxAccuracyRate = 0.15;
        //parameters->minMarkerDistanceRate = 0.3;
        cv::aruco::detectMarkers((image > i), dictPtr, corners, MarkerIDs, parameters, rejects); //detects all of the tags in the current b&w cutoff
        
        if(MarkerIDs.size() > 0)
        {
            index = -1;
            for(int i = 0; i < MarkerIDs.size(); i++) //this just checks to make sure that it found the right tag. Probably should move this into the b&w block
            {
               std::cout << i << "," << MarkerIDs[i] << "\n";
               std::cout << corners[i][1].x - corners[i][0].x << "\n\n";
                if(MarkerIDs[i] == id)
                {
                    index = i;
                    break; 
                }   
            }
            
            if(index != -1)
            {
                std::cout << "Found the correct tag!" << std::endl;
                if(writeToFile)
                {
                    mFrame = image > i; //purely for debug
                    videoWriter.write(mFrame); //purely for debug
                }    
                break;
            }
	    //else
            //    std::cout << "Found a tag but was not the correct one" << std::endl;
        }
        if(i == 220) //did not find any AR tags with any b&w cutoff
        {
            if(writeToFile)
                videoWriter.write(image);
            distanceToAR = -1;
            angleToAR = 0;
            return false;
        }
    }
    //std::cout << "got to width calc, index: " << index << std::endl;
    //for(int i = 0; i < corners[index].size(); ++i) 
    //    std::cout << corners[index][i].x << "\n";
    widthOfTag = corners[index][1].x - corners[index][0].x;
    //std::cout << "got past width calc\n" ; 
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
        parameters->adaptiveThreshWinSizeMax = 400; // These two parameters adjust the size of squares aruco breaks the image into.                                          
        parameters->adaptiveThreshWinSizeMin = 200; // If these are too low, especially min, it will detect many markers that are not there
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
