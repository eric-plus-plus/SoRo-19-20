

using namespace std;

class DriveMode
{
private:
	bool onePost;
	vector<double> getWheelSpeeds(double amountOff, double baseSpeed);
	String videoFile = "/dev/video0";
    ARTracker tracker(videoFile); //"/dev/video0"
    Location locationInst;

public:
	bool driveAlongCoordinates(vector<int[2]> locations, int id);
	bool driveAlongCoordinates(vector<int[2]> locations, int id1, int id2);
	bool trackARTag(int id);
	bool trackARTags(int id1, int id2);
	bool searchForARTag();

};

