
#include <iostream>
#include <vector>
#include <list>
#include "Location.h"
#include "DriveMode.h"
#include "ARTracker.h"

using namespace std;

vector<double> DriveMode::getWheelSpeeds(double amountOff, double baseSpeed)
{
	vector<double> PIDValues(2);
	amountOff /= .00015; //THIS IS A STUPID FIX FOR THE FIRST VERSION OF THE FORMULA. Get rid of this line if we need to retune. Same with the /15s
	if (baseSpeed > 90)
	{
		//this formula works by taking the baseSpeed and increasing or decreasing it by a percent based off of amountOff
		//this formula is still almost certainly going to need to be adjusted
		PIDValues[0] = baseSpeed + baseSpeed * (1.045443e-16 + 0.00001087878 * amountOff - 1.0889139999999999e-27 * pow(amountOff, 2) + 7.591631000000001e-17 * pow(amountOff, 3) - 7.105946999999999e-38 * pow(amountOff, 4)) / 15;
		PIDValues[1] = baseSpeed - baseSpeed * (1.045443e-16 + 0.00001087878 * amountOff - 1.0889139999999999e-27 * pow(amountOff, 2) + 7.591631000000001e-17 * pow(amountOff, 3) - 7.105946999999999e-38 * pow(amountOff, 4)) / 15;
	}

	else
	{
		PIDValues[0] = baseSpeed - baseSpeed * (1.045443e-16 + 0.00001087878 * amountOff - 1.0889139999999999e-27 * pow(amountOff, 2) + 7.591631000000001e-17 * pow(amountOff, 3) - 7.105946999999999e-38 * pow(amountOff, 4)) / 15;
		PIDValues[1] = baseSpeed + baseSpeed * (1.045443e-16 + 0.00001087878 * amountOff - 1.0889139999999999e-27 * pow(amountOff, 2) + 7.591631000000001e-17 * pow(amountOff, 3) - 7.105946999999999e-38 * pow(amountOff, 4)) / 15;
	}
	return PIDValues;
}
bool DriveMode::driveAlongCoordinates(vector<int[2]> locations, int id)
{
    ARTracker tracker;
    Location locationInst;
    float bearingTo;
    vector<double> wheelSpeeds;
    for(int i = 0; i < locations.size() && !tracker.findAR(id); ++i)
    {
         while(!tracker.findAR(id))
         {
            bearingTo = locationInst.bearingTo(locations.at(i)[0], locations.at(i)[1];
            wheelSpeeds = getWheelSpeeds(bearingTo, baseSpeed);
            //send wheel speeds
            cout << wheelSpeeds.at(0) << " : " << wheelSpeeds.at(1) << endl;
            sleep(0);
         }
         if(tracker.findAR(id))
         {
            return true;
         }
    }
    return false;
}

/*					       
for( coord in locations && !tracker.findAR(id))
    while(!tracker.findAR(id))
    {
        find bearing to face next gps location
        get wheel speeds using the previously found bearing
        send wheel speeds to wheels with communication class //just print them for now
        wait for a second
    }
}
*/
