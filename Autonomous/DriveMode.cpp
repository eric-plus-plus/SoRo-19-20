
#include <iostream>
#include <vector>
#include <math.h>
#include <string>
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
            std::string str = out->controlToStr(wheelSpeeds[0], wheelSpeeds[1], 0,0);
            out->sendMessage(&str);
            std::cout << wheelSpeeds[0] << " : " << wheelSpeeds[1] << std::endl;
            
            cv::waitKey(100); //waits for 100ms
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

bool DriveMode::trackARTag(int id)
{
    std::vector<double> wheelSpeeds;
    int timesNotFound = -1;
    //drives until the distance to the tag is less than 50cm. NOTE: rover only needs to be within 300cm to score.
    while(tracker.distanceToAR > 50 || tracker.distanceToAR == -1) //distance = -1 if the camera cannot find a tag
    {
        if(tracker.findAR(id))
        {
            std::cout << tracker.angleToAR << " " << tracker.distanceToAR << std::endl;
            wheelSpeeds = getWheelSpeeds(tracker.angleToAR, speed);
            std::cout<< round(wheelSpeeds[0]) << ", " << round(wheelSpeeds[1]) << std::endl; 
            //send wheel speeds
            std::string str = out->controlToStr(round(wheelSpeeds[0]), round(wheelSpeeds[1]), 0,0);
	    //std::cout << str << std::endl;
            out->sendMessage(&str);
            timesNotFound = 0;
        }
        else if(timesNotFound < 100 && timesNotFound != -1) //checks 100 times to ensure that the tag was lost. May want to make larger
        {
            //send wheel speeds
            std::string str = out->controlToStr(round(wheelSpeeds[0]), round(wheelSpeeds[1]), 0,0);
            std::cout<< round(wheelSpeeds[0]) << ", " << round(wheelSpeeds[1]) << std::endl;  
            out->sendMessage(&str);
            std::cout << "Didn't find it " << timesNotFound + 1 << " times" << std::endl;
            timesNotFound++;
        }
        else
        {
            //Stops the rover. Probably not the best idea but this is what we're doing for now
            std::string str = out->controlToStr(0, 0, 0,0);
            out->sendMessage(&str);
            std::cout << "Tag 1 not found" << std::endl;
        }
        cv::waitKey(10); //waits for 10ms    
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
