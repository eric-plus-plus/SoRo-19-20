#include "Location.h"
#include "gps/gps.h"
#include <iostream>

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
    char *ip = (char*)"10.0.0.222";
    char *host = (char*)"55556";
    gps_init(ip, host);
	std::thread updateThread(&Location::updateFieldsLoop, this);
	updateThread.detach();
}

void Location::stopGPSThread()
{
    running = false;
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
		std::cout << pos_llh.lat << " " << pos_llh.lon << std::endl;
	}
}

//Calculates the bearing between two points. 0 is North, 90 is East, +/-180 is South, -90 is West.
float Location::calcBearing(float lat1, float lon1, float lat2, float lon2)
{
	double x = cos(lat2 * (PI/180.0)) * sin(((double)lon2-lon1) * (PI/180.0));
	double y = cos(lat1 * (PI/180.0)) * sin(lat2 * (PI/180.0)) - sin(lat1 * (PI/180.0)) * cos(lat2 * (PI/180.0)) * cos(((double)lon2-lon1) * (PI/180.0));

	return (180.0/PI) * atan2(x,y);
}
