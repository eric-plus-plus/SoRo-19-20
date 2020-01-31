#include <vector>
#include <iostream>
#include <libs/DriveMode.h>

//takes arguments left camera file, middle camera file, right camera file
int main(int argc, char* argv[])
{    
    if(argc != 3)
        std::cout << "Arguments: main_camera_file, secondary_camera_file" << std::endl;    
    DriveMode rover(argv[1], argv[2], 30.0);
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
    return 0;
}  
