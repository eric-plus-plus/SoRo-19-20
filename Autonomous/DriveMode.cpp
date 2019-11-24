
#include <iostream>
#include <vector>
#include <string>
//#include <unistd.h>
#include "DriveMode.h"

DriveMode::DriveMode(std::string videoFile, double speed):tracker(videoFile)
{
    this->speed = speed;
}

std::vector<double> DriveMode::getWheelSpeeds(double amountOff, double baseSpeed)
{
	std::vector<double> PIDValues(2);
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

bool DriveMode::driveAlongCoordinates(std::vector<std::vector<double>> locations, int id)
{
    
    locationInst.startGPSThread();
    float bearingTo;
    std::vector<double> wheelSpeeds;
    for(int i = 0; i < locations.size(); ++i)
    {
         while(locationInst.distanceTo(locations[i][0], locations[i][1]) > 0.003)
         {
            bearingTo = locationInst.bearingTo(locations[i][0], locations[i][1]);
            wheelSpeeds = getWheelSpeeds(bearingTo, speed);
            //send wheel speeds
            //communicate.arc(wheelsSpeeds[0], wheelSpeeds[1]);
            std::string str = out->controlToStr(wheelSpeeds[0], wheelSpeeds[1], 0,0);
            out->sendMessage(&str);
            std::cout << wheelSpeeds[0] << " : " << wheelSpeeds[1] << std::endl;
            //usleep(1000000);
            cv::waitKey(100);
            if(tracker.findAR(id))
            {
                locationInst.stopGPSThread();
                return true;
            }
         }
         if(tracker.findAR(id))
         {
            locationInst.stopGPSThread();
            return true;
         }
    }
    locationInst.stopGPSThread();
    return false;
}


/*
while(distanceToAR < 300)
{
    if(findAR)
        getWheelSpeeds(angleToAR)
}
*/
bool DriveMode::trackARTag(int id)
{
    //ARTracker tracker;
    std::vector<double> wheelSpeeds;
    
    while(tracker.distanceToAR > 300 || tracker.distanceToAR == -1)
    {
        if(tracker.findAR(id))
        {
	    std::cout << tracker.angleToAR << " " << tracker.distanceToAR << std::endl;
            wheelSpeeds = getWheelSpeeds(tracker.angleToAR, speed);
            //send wheel speeds
            //communicate.arc(wheelSpeeds[0], wheelSpeeds[1]);
            std::string str = out->controlToStr(wheelSpeeds[0], wheelSpeeds[1], 0,0);
            out->sendMessage(&str);
            //usleep(1000000);
            cv::waitKey(1000);
        }
//	else
//	    std::cout << "cannot find tag" << std::endl;
    }
    return true;
}



/*bool DriveMode::driveAlongCoordinates(vector<int[2]> locations, int id1, int id2)
{
    ARTracker tracker;
    Location locationInst;
    float bearingTo;
    vector<double> wheelSpeeds;
    for(int i = 0; i < locations.size(); ++i)
    {
         while(locationInst.distanceTo(locations.at(i)[0], locations.at(i)[1]) > 0.003)
         {
            bearingTo = locationInst.bearingTo(locations.at(i)[0], locations.at(i)[1]);
            wheelSpeeds = getWheelSpeeds(bearingTo, baseSpeed);
            //send wheel speeds
            cout << wheelSpeeds.at(0) << " : " << wheelSpeeds.at(1) << endl;
            sleep(0);
            if(tracker.findAR(id1) && tracker.findAR(id2))
            {
                return true;
            }
         }
         if(tracker.findAR(id1) && tracker.findAR(id2))
         {
            return true;
         }
    }
    return false;
}*/
