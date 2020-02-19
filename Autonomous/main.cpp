#include <vector>
#include <iostream>
#include <libs/DriveMode.h>

void driveToPoll(int id, DriveMode rover)
{
    double lat, lon;
    std::string ledStr;
    std::vector<std::vector<double>> locations; 
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
    rover.driveAlongCoordinates(locations, id);
    rover.trackARTag(id);
    ledStr = rover.out->ledToStr(false, true, false);
    rover.out->sendMessage(&ledStr); //green
}

//takes arguments mainCamera, rest of the camera files
int main(int argc, char* argv[])
{        
    DriveMode rover(argv + 1, "MJPG", 40.0);
    std::string ledStr = rover.out->ledToStr(true, false, false);
    rover.out->sendMessage(&ledStr); //red
    
    driveToPoll(0, rover);
    driveToPoll(4, rover);
    driveToPoll(5, rover);

    std::cout << "Finished everything!" << std::endl;
    std::cout << "\ndon't worry about the next error" << std::endl;
    rover.locationInst.stopGPS();
    return 0;
}  
