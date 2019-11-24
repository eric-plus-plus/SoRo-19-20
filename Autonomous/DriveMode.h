#include "ARTracker.h"
#include "Location.h"
#include "UDPOut.h"
class DriveMode
{
private:
    double speed;
	bool onePost;
	std::vector<double> getWheelSpeeds(double amountOff, double baseSpeed);
    ARTracker tracker; //"/dev/video0"
    Location locationInst;
    //Communication communicate;
    UDPOut* out = new UDPOut("192.168.1.100", 1256, "192.168.1.106", 1001);

public:

    DriveMode(std::string videoFile, double speed);
	bool driveAlongCoordinates(std::vector<std::vector<double>> locations, int id);
	bool driveAlongCoordinates(std::vector<std::vector<double>> locations, int id1, int id2);
	bool trackARTag(int id);
	bool trackARTags(int id1, int id2);
	bool searchForARTag();

};

