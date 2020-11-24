#include "ARTracker.h"
#include "Location.h"
#include "UDPOut.h"
class DriveMode
{
private:
    double speed;
    double* leftWheelSpeed;
    double* rightWheelSpeed;
    int jetsonPort, nanoPort;
    bool onePost, running;
    std::string speedString, jetsonIP, nanoIP;
    std::vector<double> getWheelSpeeds(double error, double baseSpeed); //returns the speeds the wheels should be using at based on the given error with PID algorithms
    ARTracker tracker;

    //variables to be used for the i in getWheelSpeeds
    double errorAccumulation, time; //time in ms
    
    void sendSpeed(); //for the thread to send wheel speeds
    void printSpeeds(); //prints the speeds. Just to be lazy with
    bool config();	

public:
    Location locationInst;
    DriveMode(char* cameras[], std::string format, double speed); //the video file of the camera and the base speed that the rover drives at. Also starts a thread to send speed
    bool driveAlongCoordinates(std::vector<std::vector<double>> locations, int id); //drives along a list of coordinates until it gets to the end or finds the given AR tag
    bool driveAlongCoordinates(std::vector<std::vector<double>> locations, int id1, int id2); //same as above except it looks for 2 tags for the gates
    bool trackARTag(int id); //tracks the AR tag here. NOTE: really need to have found the ar tag first
    bool trackARTags(int id1, int id2); //same as above except it tracks the center of the posts between the tags
    bool searchForARTag(); //searches for the tags if it gets to the end of driveAlongCoordinates without finding one
	
    UDPOut* out; //ip of the jetson, ip of the nano
};

