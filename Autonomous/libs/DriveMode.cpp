
#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include "DriveMode.h"

DriveMode::DriveMode(char* cameras[], std::string format, double speed):tracker(cameras, format)
{
    running = true;
    this->speed = speed; //probably going to want more ways to change the speed...
    std::thread speedThread(sendSpeed, this);
}

//Sends speed in a separate thread
void DriveMode::sendSpeed() 
{
    while(running) 
    {
	//send wheel speeds
        std::string str = out->controlToStr(round(leftWheelSpeed), round(rightWheelSpeed), 0,0);
        out->sendMessage(&str);
        std::cout << round(leftWheelSpeed) << " : " << round(rightWheelSpeed) << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::vector<double> DriveMode::getWheelSpeeds(double error, double baseSpeed)
{
	std::vector<double> PIDValues(2);
	
	/*error /= .00015; //THIS IS A STUPID FIX FOR THE FIRST VERSION OF THE FORMULA. Get rid of this line if we need to retune. Same with the /15s
	if (baseSpeed < 0)
	{
		//this formula works by taking the baseSpeed and increasing or decreasing it by a percent based off of error
		//this formula is still almost certainly going to need to be adjusted
		PIDValues[0] = baseSpeed + baseSpeed * (1.045443e-16 + 0.00001087878 * error - 1.0889139999999999e-27 * pow(error, 2) + 7.591631000000001e-17 * pow(error, 3) - 7.105946999999999e-38 * pow(error, 4)) / 15;
		PIDValues[1] = baseSpeed - baseSpeed * (1.045443e-16 + 0.00001087878 * error - 1.0889139999999999e-27 * pow(error, 2) + 7.591631000000001e-17 * pow(error, 3) - 7.105946999999999e-38 * pow(error, 4)) / 15;
	}

	else
	{
		PIDValues[0] = baseSpeed - baseSpeed * (1.045443e-16 + 0.00001087878 * error - 1.0889139999999999e-27 * pow(error, 2) + 7.591631000000001e-17 * pow(error, 3) - 7.105946999999999e-38 * pow(error, 4)) / 15;
		PIDValues[1] = baseSpeed + baseSpeed * (1.045443e-16 + 0.00001087878 * error - 1.0889139999999999e-27 * pow(error, 2) + 7.591631000000001e-17 * pow(error, 3) - 7.105946999999999e-38 * pow(error, 4)) / 15;
	}*/
	
	double kp = .5, ki = .0005;
	errorAccumulation += error * time;
	PIDValues[0] = speed - (error * kp + errorAccumulation * ki);
	PIDValues[1] = speed + (error * kp + errorAccumulation * ki);

	int max = 75;
    if(PIDValues[0] > max) PIDValues[0] = max;
    if(PIDValues[1] > max) PIDValues[1] = max;
    if(PIDValues[0] < -max) PIDValues[0] = -max;
    if(PIDValues[1] < -max) PIDValues[1] = -max;
	return PIDValues;
}

bool DriveMode::driveAlongCoordinates(std::vector<std::vector<double>> locations, int id) //used for legs 1-3
{    
    locationInst.startGPSThread();

    std::cout<<"Waiting for GPS connection..." << std::endl;
    while(locationInst.allZero); //waits for the GPS to pick something up before starting
    std::cout << "Connected to GPS" << std::endl; 
     
    //Drives for 2 seconds to hopefully get a good angle from the gps
    std::string str = out->controlToStr(speed,speed,0,0);
    for(int i = 0; i < 40; i++)
    {
        out->sendMessage(&str);
        cv::waitKey(100);
    }
    
    float bearingTo;
    std::vector<double> wheelSpeeds;
    for(int i = 0; i < locations.size(); ++i)
    {
        time = 0;
        while(locationInst.distanceTo(locations[i][0], locations[i][1]) > 0.001) //.001km
        {
            bearingTo = locationInst.bearingTo(locations[i][0], locations[i][1]);
            wheelSpeeds = getWheelSpeeds(bearingTo, speed);
	    leftWheelSpeed = wheelSpeeds[1];
	    rightWheelSpeed = wheelSpeeds[0];            

            cv::waitKey(10); //waits for 10ms although the actual wait may be a bit longer
            time += 10;
            if(tracker.findAR(id))
            {
                locationInst.stopGPSThread();
		running = false;
                return true;
            }
        }
        if(tracker.findAR(id))
        {
            locationInst.stopGPSThread();
	    running = false;
            return true;
        }
    }
    locationInst.stopGPSThread();
    running = false;
    return false; //got to gps location without finding the wanted ar tag
}

bool DriveMode::trackARTag(int id) //used for legs 1-3
{
    std::string str;
    std::vector<double> wheelSpeeds;
    int timesNotFound = 0;
    int stopDistance = 250;  //drives until the distance to the tag is less than stopDistance in cm. NOTE: rover only needs to be within 300cm to score.
    
    tracker.trackAR(id); //gets an intial angle from the main camera
    
    //turns to face the artag directly before driving to it. May want to convert to PID although this also shouldn't have to be super accurate.
    while(tracker.angleToAR > 5 || tracker.angleToAR < -5 || tracker.angleToAR == 0) //its 0 if it doesn't see it
    {
        if(tracker.trackAR(id))
        {
            if(tracker.angleToAR > 5)
            {
                str = out->controlToStr(30, -30, 0,0);
            }
            else
            {
                str = out->controlToStr(-30, 30, 0,0);
            }
            std::cout << tracker.angleToAR << " " << tracker.distanceToAR << std::endl;
            out->sendMessage(&str);
            timesNotFound = 0;
        }
        else //turns to the left until it sees it
        {
            str = out -> controlToStr(-30, 30, 0, 0);
            out -> sendMessage(&str);
        }
        cv::waitKey(10);
    }
    
    while(tracker.distanceToAR > stopDistance || tracker.distanceToAR == -1) //distance = -1 if the camera cannot find a tag
    {
        time = 0;
        if(tracker.trackAR(id) || timesNotFound < 100 && timesNotFound != -1)
        {
            if(tracker.trackAR(id))
            {
                wheelSpeeds = getWheelSpeeds(tracker.angleToAR, speed);
                timesNotFound = 0;
            }
            else
            {
                std::cout << "Didn't find it " << timesNotFound + 1 << " times" << std::endl;
                timesNotFound++;
            }
            std::cout << tracker.angleToAR << " " << tracker.distanceToAR << std::endl;
            std::cout<< round(wheelSpeeds[1]) << ", " << round(wheelSpeeds[0]) << std::endl; 
            //send wheel speeds
            str = out->controlToStr(round(wheelSpeeds[1]), round(wheelSpeeds[0]), 0,0);
            out->sendMessage(&str);
            
        }
        else
        {
            //Stops the rover
            str = out->controlToStr(0, 0, 0,0);
            out->sendMessage(&str);
            std::cout << "Tag not found" << std::endl;
            return false; //TODO: do something about this
        }
        cv::waitKey(10); //waits for 10ms    
        time += 10;
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
