#include <vector>
#include <iostream>
#include <libs/DriveMode.h>

//takes arguments mainCamera, rest of the camera files
int main(int argc, char* argv[])
{    
    std::string ledStr;
    DriveMode rover(argv + 1, "YUYV", 40.0);
    
    ledStr = rover.out->ledToStr(true, false, false);
    rover.out->sendMessage(&ledStr); //red
    std::vector<std::vector<double>> locations;
    double lat, lon;
    while(true)
    {
        std::vector<double> point;
        std::cout<<"Enter lat and lon: " << std::endl;
        std::cin >> lat;
        std::cin >> lon;
        if(lat == -1 && lon == -1)
            break;
        point.push_back(lat);
        point.push_back(lon);
        locations.push_back(point);
    }
    rover.driveAlongCoordinates(locations, 5);
    rover.trackARTag(5);
    ledStr = rover.out->ledToStr(false, true, false);
    rover.out->sendMessage(&ledStr); //green
   
    locations.clear(); 
    while(true)
    {
        std::vector<double> point;
        std::cout<<"Enter lat and lon: " << std::endl;
        std::cin >> lat;
        std::cin >> lon;
        if(lat == -1 && lon == -1)
            break;
        point.push_back(lat);
        point.push_back(lon);
        locations.push_back(point);
    }
    ledStr = rover.out->ledToStr(true, false, false);
    rover.out->sendMessage(&ledStr); //red
    rover.driveAlongCoordinates(locations, 0);
    rover.trackARTag(0);
    ledStr = rover.out->ledToStr(false, true, false);
    rover.out->sendMessage(&ledStr); //green
    
    
    std::cout << "Finished everything!" << std::endl;
    std::cout << "\ndon't worry about the next error" << std::endl;
    rover.locationInst.stopGPS();
    return 0;
}  
