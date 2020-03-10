#include "Location.h"
#include "gps/gps.h"
#include <iostream>
#include <vector>

//Reads the file and sets the variables need in the class
bool Location::config() {
    std::ifstream file;
    std::string line, info;
	std::vector<std::string> lines;
    file.open("../config.txt");
    if(!file.is_open())
		return false;
    while(getline(file, line)) 
    {
		//info += line;
		lines.push_back(line);
	}
	for(int i = 0; i < lines.size(); ++i) 
	{
		if(lines[i].find("SWIFT_IP=") != std::string::npos) 
			swiftIP = lines[i].substr(lines[i].find("SWIFT_IP=") + 9);
		if(lines[i].find("SWIFT_PORT=") != std::string::npos) 
			swiftPort = lines[i].substr(lines[i].find("SWIFT_PORT=") + 11);
	}
	return true;
}

//Returns distance in kilometers between current latitude and longitude and parameters
float Location::distanceTo(float lat, float lon)
{
	float earthRadius = 6371.301;
	float deltaLat = (lat - latitude) * (PI/180.0);
	float deltaLon = (lon - longitude) * (PI/180.0);

	double a = sin((deltaLat)/2) * sin((deltaLat)/2) + 
		cos(latitude * (PI/180.0)) * cos(lat * (PI/180.0)) * sin(deltaLon/2) * sin(deltaLon/2);
	double c = 2 * atan2(sqrt(a), sqrt(1-a));
	return earthRadius * c;
}

//Calculates difference between bearing to the parameters and current bearing.
//Positive is turn right, negative is turn left
float Location::bearingTo(float lat, float lon)
{
	float resultBearing = calcBearing(latitude, longitude, lat, lon) - bearing;
	if (resultBearing < -180)
	{
		resultBearing += 360;
	}
	if (resultBearing > 180)
	{
		resultBearing -= 360;
	}
	return resultBearing;
}

//Starts updating public fields from the GPS box
void Location::startGPSThread()
{
    running = true;
	std::thread updateThread(&Location::updateFieldsLoop, this);
	updateThread.detach();
}

void Location::stopGPSThread()
{
    running = false;
}

void Location::startGPS()
{
	if(!config())
		std::cout << "Error opening file" << std::endl;    
	char* ip = &swiftIP[0];
    char* host = &swiftPort[0];
    gps_init(ip, host);
}

void Location::stopGPS()
{
    running = false;
    gps_finish();
}

//Updates all fields from the auto-filled struct defined in navigation.h
void Location::updateFieldsLoop()
{
	while(running)
	{
		if(pos_llh.lat + pos_llh.lon != 0)
		{
			oldLatitude = latitude;
			oldLongitude = longitude;
		
			latitude = pos_llh.lat;
			longitude = pos_llh.lon;
			height = pos_llh.height;
			time = pos_llh.tow;
			error = (pos_llh.h_accuracy + pos_llh.v_accuracy) / 2.0;
			bearing = calcBearing(oldLatitude, oldLongitude, latitude, longitude);

			allZero = false;
		}
		else allZero = true;
		std::this_thread::sleep_for(std::chrono::seconds(waitDuration));
		std::cout << "Lat: " <<  pos_llh.lat << ", Lon: " << pos_llh.lon << std::endl;
	}
}

//Calculates the bearing between two points. 0 is North, 90 is East, +/-180 is South, -90 is West.
float Location::calcBearing(float lat1, float lon1, float lat2, float lon2)
{
	double x = cos(lat2 * (PI/180.0)) * sin(((double)lon2-lon1) * (PI/180.0));
	double y = cos(lat1 * (PI/180.0)) * sin(lat2 * (PI/180.0)) - sin(lat1 * (PI/180.0)) * cos(lat2 * (PI/180.0)) * cos(((double)lon2-lon1) * (PI/180.0));

	return (180.0/PI) * atan2(x,y);
}
