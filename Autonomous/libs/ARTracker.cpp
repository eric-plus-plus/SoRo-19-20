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
    std::vector<int> row01 = {1,1,0,1,1};
    std::vector<int> row02 = {1,1,0,1,1};
    std::vector<int> row03 = {1,0,1,0,1};
    std::vector<int> row04 = {1,1,1,1,1};
    std::vector<int> row05 = {1,1,1,1,1};
    cv::Mat markerBits0 = cv::Mat(0, 0, CV_8UC1);
    markerBits0.push_back(row01);
    markerBits0.push_back(row02);
    markerBits0.push_back(row03);
    markerBits0.push_back(row04);
    markerBits0.push_back(row05);
    cv::Mat markerCompressed0 = cv::aruco::Dictionary::getByteListFromBits(markerBits0);
    urcDict.bytesList.push_back(markerCompressed0);

    std::vector<int> row11 = {1,1,0,1,1};
    std::vector<int> row12 = {1,1,0,1,1};
    std::vector<int> row13 = {1,0,1,0,1};
    std::vector<int> row14 = {0,0,1,1,0};
    std::vector<int> row15 = {1,1,1,0,1};
    cv::Mat markerBits1 = cv::Mat(0, 0, CV_8UC1);
    markerBits1.push_back(row11);
    markerBits1.push_back(row12);
    markerBits1.push_back(row13);
    markerBits1.push_back(row14);
    markerBits1.push_back(row15);
    cv::Mat markerCompressed1 = cv::aruco::Dictionary::getByteListFromBits(markerBits1);
    urcDict.bytesList.push_back(markerCompressed1);

    std::vector<int> row21 = {1,1,0,1,1};
    std::vector<int> row22 = {1,1,0,1,1};
    std::vector<int> row23 = {1,0,1,0,1};
    std::vector<int> row24 = {1,0,1,1,0};
    std::vector<int> row25 = {1,0,1,1,0};
    cv::Mat markerBits2 = cv::Mat(0, 0, CV_8UC1);
    markerBits2.push_back(row21);
    markerBits2.push_back(row22);
    markerBits2.push_back(row23);
    markerBits2.push_back(row24);
    markerBits2.push_back(row25);
    cv::Mat markerCompressed2 = cv::aruco::Dictionary::getByteListFromBits(markerBits2);
    urcDict.bytesList.push_back(markerCompressed2);

    std::vector<int> row31 = {1,1,0,1,1};
    std::vector<int> row32 = {1,1,0,1,1};
    std::vector<int> row33 = {1,0,1,0,1};
    std::vector<int> row34 = {0,1,1,1,1};
    std::vector<int> row35 = {1,0,1,0,0};
    cv::Mat markerBits3 = cv::Mat(0, 0, CV_8UC1);
    markerBits3.push_back(row31);
    markerBits3.push_back(row32);
    markerBits3.push_back(row33);
    markerBits3.push_back(row34);
    markerBits3.push_back(row35);
    cv::Mat markerCompressed3 = cv::aruco::Dictionary::getByteListFromBits(markerBits3);
    urcDict.bytesList.push_back(markerCompressed3);

    std::vector<int> row41 = {1,1,0,1,1};
    std::vector<int> row42 = {1,1,0,1,1};
    std::vector<int> row43 = {1,0,1,0,1};
    std::vector<int> row44 = {0,1,1,1,0};
    std::vector<int> row45 = {0,1,1,1,0};
    cv::Mat markerBits4 = cv::Mat(0, 0, CV_8UC1);
    markerBits4.push_back(row41);
    markerBits4.push_back(row42);
    markerBits4.push_back(row43);
    markerBits4.push_back(row44);
    markerBits4.push_back(row45);
    cv::Mat markerCompressed4 = cv::aruco::Dictionary::getByteListFromBits(markerBits4);
    urcDict.bytesList.push_back(markerCompressed4);

    std::vector<int> row51 = {1,1,0,1,1};
    std::vector<int> row52 = {1,1,0,1,1};
    std::vector<int> row53 = {1,0,1,0,1};
    std::vector<int> row54 = {1,0,1,1,1};
    std::vector<int> row55 = {0,1,1,0,0};
    cv::Mat markerBits5 = cv::Mat(0, 0, CV_8UC1);
    markerBits5.push_back(row51);
    markerBits5.push_back(row52);
    markerBits5.push_back(row53);
    markerBits5.push_back(row54);
    markerBits5.push_back(row55);
    cv::Mat markerCompressed5 = cv::aruco::Dictionary::getByteListFromBits(markerBits5);
    urcDict.bytesList.push_back(markerCompressed5);

    std::vector<int> row61 = {1,1,0,1,1};
    std::vector<int> row62 = {1,1,0,1,1};
    std::vector<int> row63 = {1,0,1,0,1};
    std::vector<int> row64 = {0,0,1,1,1};
    std::vector<int> row65 = {0,0,1,1,1};
    cv::Mat markerBits6 = cv::Mat(0, 0, CV_8UC1);
    markerBits6.push_back(row61);
    markerBits6.push_back(row62);
    markerBits6.push_back(row63);
    markerBits6.push_back(row64);
    markerBits6.push_back(row65);
    cv::Mat markerCompressed6 = cv::aruco::Dictionary::getByteListFromBits(markerBits6);
    urcDict.bytesList.push_back(markerCompressed6);

    std::vector<int> row71 = {1,1,0,1,1};
    std::vector<int> row72 = {1,1,0,1,1};
    std::vector<int> row73 = {1,0,1,0,1};
    std::vector<int> row74 = {1,1,1,1,0};
    std::vector<int> row75 = {0,0,1,0,1};
    cv::Mat markerBits7 = cv::Mat(0, 0, CV_8UC1);
    markerBits7.push_back(row71);
    markerBits7.push_back(row72);
    markerBits7.push_back(row73);
    markerBits7.push_back(row74);
    markerBits7.push_back(row75);
    cv::Mat markerCompressed7 = cv::aruco::Dictionary::getByteListFromBits(markerBits7);
    urcDict.bytesList.push_back(markerCompressed7);

    std::vector<int> row81 = {1,1,0,1,1};
    std::vector<int> row82 = {1,1,0,1,1};
    std::vector<int> row83 = {1,0,1,0,1};
    std::vector<int> row84 = {0,0,1,0,1};
    std::vector<int> row85 = {1,1,1,1,0};
    cv::Mat markerBits8 = cv::Mat(0, 0, CV_8UC1);
    markerBits8.push_back(row81);
    markerBits8.push_back(row82);
    markerBits8.push_back(row83);
    markerBits8.push_back(row84);
    markerBits8.push_back(row85);
    cv::Mat markerCompressed8 = cv::aruco::Dictionary::getByteListFromBits(markerBits8);
    urcDict.bytesList.push_back(markerCompressed8);

    std::vector<int> row91 = {1,1,0,1,1};
    std::vector<int> row92 = {1,1,0,1,1};
    std::vector<int> row93 = {1,0,1,0,1};
    std::vector<int> row94 = {1,1,1,0,0};
    std::vector<int> row95 = {1,1,1,0,0};
    cv::Mat markerBits9 = cv::Mat(0, 0, CV_8UC1);
    markerBits9.push_back(row91);
    markerBits9.push_back(row92);
    markerBits9.push_back(row93);
    markerBits9.push_back(row94);
    markerBits9.push_back(row95);
    cv::Mat markerCompressed9 = cv::aruco::Dictionary::getByteListFromBits(markerBits9);
    urcDict.bytesList.push_back(markerCompressed9);

    std::vector<int> row101 = {1,1,0,1,1};
    std::vector<int> row102 = {1,1,0,1,1};
    std::vector<int> row103 = {1,0,1,0,1};
    std::vector<int> row104 = {0,1,1,0,0};
    std::vector<int> row105 = {1,0,1,1,1};
    cv::Mat markerBits10 = cv::Mat(0, 0, CV_8UC1);
    markerBits10.push_back(row101);
    markerBits10.push_back(row102);
    markerBits10.push_back(row103);
    markerBits10.push_back(row104);
    markerBits10.push_back(row105);
    cv::Mat markerCompressed10 = cv::aruco::Dictionary::getByteListFromBits(markerBits10);
    urcDict.bytesList.push_back(markerCompressed10);

    // PRINT DICTIONARY FOR FURTHER USE
    cv::FileStorage fs("../urcDict.yml", cv::FileStorage::WRITE);
    fs << "MarkerSize" << urcDict.markerSize;
    fs << "MaxCorrectionBits" << urcDict.maxCorrectionBits;
    fs << "ByteList" << urcDict.bytesList;
    fs.release();

    for(int i = 0; true; i++) //initializes the cameras
    {
        if(cameras[i] == NULL)
            break;
        caps.push_back(new cv::VideoCapture(cameras[i], cv::CAP_V4L2));
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
    
    //MDetector.setDictionary("../urc.dict");
}

bool ARTracker::arFound(int id, cv::Mat image, bool writeToFile)
{
    cv::cvtColor(image, image, cv::COLOR_RGB2GRAY); //converts to grayscale
    
    //tries converting to b&w using different different cutoffs to find the perfect one for the current lighting
    for(int i = 40; i <= 220; i+=60)
    {
        cv::aruco::detectMarkers((image > i), &urcDict, corners, MarkerIDs); //detects all of the tags in the current b&w cutoff
        if(MarkerIDs.size() > 0)
        {
            if(writeToFile)
            {
		        mFrame = image > i; //purely for debug
                videoWriter.write(mFrame); //purely for debug
            }    
            break;
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

    int index = -1;
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
        distanceToAR=-1;
        angleToAR=0;
        std::cout << "Found a tag but was not the correct one" << std::endl;
        return false; //correct ar tag not found
    }  

    else
    {
        
        widthOfTag = corners[index][1].x - corners[index][0].x;
        //distanceToAR = (knownWidthOfTag(20cm) * focalLengthOfCamera) / pixelWidthOfTag
        distanceToAR = (knownTagWidth * focalLength) / widthOfTag;
        
        centerXTag = (corners[index][1].x + corners[index][0].x) / 2;
        angleToAR = degreesPerPixel * (centerXTag - 960); //takes the pixels from the tag to the center of the image and multiplies it by the degrees per pixel
        
        return true;
    }
}

int ARTracker::countValidARs(int id1, int id2, cv::Mat image, bool writeToFile)
{
    cv::cvtColor(image, image, cv::COLOR_RGB2GRAY); //converts to grayscale
    
    //tries converting to b&w using different different cutoffs to find the perfect one for the ar tag
    for(int i = 40; i <= 220; i+=60)
    {
        cv::aruco::detectMarkers((image > i), &urcDict, corners, MarkerIDs);
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
        distanceToAR1 = (knownTagWidth * focalLength) / widthOfTag;
        distanceToAR2 = (knownTagWidth * focalLength) / widthOfTag;
        distanceToAR = (distanceToAR1 + distanceToAR2) / 2;
        
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
