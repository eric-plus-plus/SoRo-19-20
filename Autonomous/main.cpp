#include <vector>
#include <iostream>
#include <libs/DriveMode.h>

//id of the tag, a DriveMode object, and whether or not the rover should search for a tag if it doesn't see before arriving at the GPS coords
//finish should be false for the first two legs where the GPS coords are right at the AR tag, true otherwise
void driveToPoll(int id, DriveMode *rover, bool finish)
{
    double lat, lon;
    std::string ledStr;
    bool found;
    std::vector<std::vector<double>> locations; 
    
    //gets lists of coords to drive to
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
    
    ledStr = rover->out->ledToStr(true, false, false);
    rover->out->sendMessage(&ledStr); //red
    
    found = rover->driveAlongCoordinates(locations, id);
    //only tracks the tag if finish is true or if it saw it while driving to the GPS coords
    if(found || finish) 
        rover->trackARTag(id);
    
    ledStr = rover->out->ledToStr(false, true, false);
    rover->out->sendMessage(&ledStr); //green
}

//for the gates
//takes the two ids of the gates and a DriveMode object
void driveToPolls(int id1,int id2, DriveMode* rover)
{
    double lat, lon;
    std::string ledStr;
    std::vector<std::vector<double>> locations; 
    
    //gets the coords to drive along
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
    
    ledStr = rover->out->ledToStr(true, false, false);
    rover->out->sendMessage(&ledStr); //red
    
    rover->driveAlongCoordinates(locations, id1, id2);
    //always tracks for the gates because we have to drive through them to get points
    rover->trackARTags(id1,id2);
    
    ledStr = rover->out->ledToStr(false, true, false);
    rover->out->sendMessage(&ledStr); //green
}
 
//takes arguments mainCamera, rest of the camera files
int main(int argc, char* argv[])
{
    DriveMode rover(argv + 1, "MJPG", 40.0);
    std::string ledStr = rover.out->ledToStr(true, false, false);
    rover.out->sendMessage(&ledStr); //red
    
    driveToPoll(0, &rover, false);
    driveToPoll(5, &rover, false);
    driveToPoll(0, &rover, true);
    
    driveToPolls(4,5,&rover);  
  
    std::cout << "Finished everything!" << std::endl;
    std::cout << "\ndon't worry about the next error" << std::endl;
    rover.locationInst.stopGPS(); //just crashes the program even though its supposed to clean up the GPS threads
    return 0;
}

 
