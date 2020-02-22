#include <vector>
#include <iostream>
#include <fstream>
#include <libs/DriveMode.h>

//Reads the config file and takes only the variables needed 
bool config(std::string* format, double* speed) {
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
	*speed = std::stod(info.substr(info.find("SPEED=") + 6, 2));
	*format = info.substr(info.find("FORMAT=") + 7, 4);
	return true;
}   
//takes arguments mainCamera, rest of the camera files
int main(int argc, char* argv[])
{    
    std::string ledStr, format; 
	double speed;

	if(!config(&format, &speed))
		std::cout << "Error opening file" << std::endl;
	
    DriveMode rover(argv + 1, format, speed);
    
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
    rover.driveAlongCoordinates(locations, 4);
    rover.trackARTag(4);
    ledStr = rover.out->ledToStr(false, true, false);
    rover.out->sendMessage(&ledStr); //green

    std::cout << "Finished everything!" << std::endl;
    std::cout << "\ndon't worry about the next error" << std::endl;
    rover.locationInst.stopGPS();
    return 0;
}  
