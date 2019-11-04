#include <iostream>
#include "Location.h"

#include <chrono>

using namespace std;

int main()
{
	Location loc;
	/*loc.latitude = -85.625;
	loc.longitude = 145.21;
	loc.bearing = -170.235;
	float externLat = -77.278;
	float externLong = 170.521;

	//DistanceTo test
	//Expected: 996.4 km
	cout << loc.distanceTo(externLat, externLong) << endl;

	//BearingTo test
	//Expected: 37.18918590498765 + 170.235 = 207.424185905 - 360 = -152.576
	cout << loc.bearingTo(externLat, externLong) << endl;*/
	loc.startGPSThread();
	
	while(true)
	{
	    cout << "Lat: " << loc.latitude << std::endl;
	    cout << "Lon: " << loc.longitude << std::endl;
	    cout << "Height: " << loc.height << std::endl;
	    cout << "Time: " << loc.time << std::endl;
	    cout << "Error: " << loc.error << std::endl;
	    cout << "Bearing: " << loc.bearing << std::endl << std::endl;
	    
	    std::this_thread::sleep_for(std::chrono::seconds(2));
	
	}


}
