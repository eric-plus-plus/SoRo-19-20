#ifndef LOCATION_H
#define LOCATION_H


#include <math.h>
#include <thread>
#include <chrono>

#define PI 3.14159265358979323846

class Location
{
private:
	float oldLatitude = 0.0;
	float oldLongitude = 0.0;
	bool running = true;

	float calcBearing(float lat1, float lon1, float lat2, float lon2);
	void updateFieldsLoop();

public:
	
	float latitude = 0.0;
	float longitude = 0.0;
	float height = 0.0;
	//tow defined in msg_pos_llh_t in navigation.h
	unsigned long time = 0;
	//average of horizontal and vertical accuracies defined in msg_pos_llh_t in navigation.h
	unsigned long error = 0;
	//Current bearing of the rover: -180 south to -90 west to 180 south
	float bearing = 0.0;

	//Returns distance in kilometers between current latitude and longitude and parameters
	float distanceTo(float lat, float lon);
	//Calculates difference between bearing to the parameters and current bearing.
	//Positive is turn right, negative is turn left
	float bearingTo(float lat, float lon);
	//Start updating this object's fields every 1 second
	void startGPSThread();
};

#endif
