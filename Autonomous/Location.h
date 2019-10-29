#ifndef LOCATION_H
#define LOCATION_H

#include "gps/gps.h"
#include <math.h>
#include <thread>
#include <chrono>

#define PI 3.14159265358979323846

class Location
{
private:
	float oldLatitude;
	float oldLongitude;
	bool running = true;

	float calcBearing(float lat1, float lon1, float lat2, float lon2);
	void updateFieldsLoop();

public:
	
	float latitude;
	float longitude;
	float height;
	//tow defined in msg_pos_llh_t in navigation.h
	unsigned long time;
	//average of horizontal and vertical accuracies defined in msg_pos_llh_t in navigation.h
	unsigned long error;
	//Current bearing of the rover: -180 south to -90 west to 180 south
	float bearing;

	//Returns distance in kilometers between current latitude and longitude and parameters
	float distanceTo(float lat, float lon);
	//Calculates difference between bearing to the parameters and current bearing.
	//Positive is turn right, negative is turn left
	float bearingTo(float lat, float lon);
	//Start updating this object's fields every 1 second
	void startGPSThread();
};

#endif
