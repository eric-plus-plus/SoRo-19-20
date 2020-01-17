#include <vector>
#include <iostream>
#include <libs/DriveMode.h>

int main()
{
    
    DriveMode rover("/dev/video1", 60.0);
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
    rover.driveAlongCoordinates(locations, 0);
    rover.trackARTag(0);
    return 0;
}  
