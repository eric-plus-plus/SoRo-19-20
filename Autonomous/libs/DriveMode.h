#include "ARTracker.h"
#include "Location.h"
#include "UDPOut.h"
class DriveMode
{
private:
    double speed;
	bool onePost;
	std::vector<double> getWheelSpeeds(double amountOff, double baseSpeed);
    ARTracker tracker;
    UDPOut* out = new UDPOut("10.0.0.2", 1256, "10.0.0.101", 1001); //ip of the jetson, ip of the nano

public:
    Location locationInst;
    DriveMode(std::string leftFile, std::string middleFile, std::string rightFile, double speed); //the video file of the camera and the base speed that the rover drives at
	bool driveAlongCoordinates(std::vector<std::vector<double>> locations, int id); //drives along a list of coordinates until it gets to the end or finds the given AR tag
	bool driveAlongCoordinates(std::vector<std::vector<double>> locations, int id1, int id2); //same as above except it looks for 2 tags for the gates
	bool trackARTag(int id); //tracks the AR tag here. NOTE: really need to have found the ar tag first
	bool trackARTags(int id1, int id2); //same as above except it tracks the center of the posts between the tags
	bool searchForARTag(); //searches for the tags if it gets to the end of driveAlongCoordinates without finding one

};

