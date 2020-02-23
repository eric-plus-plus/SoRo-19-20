
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string>
#include "DriveMode.h"

//Reads the file and sets the variables need in the class
bool DriveMode::config() 
{
    std::ifstream file;
    std::string line;//, info;
	std::vector<std::string> lines;
    file.open("config.txt");
    if(!file.is_open())
		return false;
    while(getline(file, line)) 
    {
		//info += line;
		lines.push_back(line);
	}
	for(int i = 0; i < lines.size(); ++i) 
	{
		if(lines[i].find("JETSON_IP=") != std::string::npos) 
			jetsonIP = lines[i].substr(lines[i].find("JETSON_IP=") + 10).c_str();
		if(lines[i].find("JETSON_PORT=") != std::string::npos) 
			jetsonPort = std::stoi(lines[i].substr(lines[i].find("JETSON_PORT=") + 12));
		if(lines[i].find("NANO_IP=") != std::string::npos) 
			nanoIP = lines[i].substr(lines[i].find("NANO_IP=") + 8).c_str();
		if(lines[i].find("NANO_PORT=") != std::string::npos) 
			nanoPort = std::stoi(lines[i].substr(lines[i].find("NANO_PORT=") + 10));
	}
	/*//The numbers there will correctly parse the proper sized substring
	jetsonIP = info.substr(info.find("JETSON_IP=") + 10, 8).c_str();
	jetsonPort = std::stoi(info.substr(info.find("JETSON_PORT=") + 12, 5));
	nanoIP = info.substr(info.find("NANO_IP=") + 8, 10).c_str();
	nanoPort = std::stoi(info.substr(info.find("NANO_PORT=") + 10, 5));
	*/return true;
}  

DriveMode::DriveMode(char* cameras[], std::string format, double speed):tracker(cameras, format)
{
	if(!config())
		std::cout << "Error opening file" << std::endl;
	out = new UDPOut(jetsonIP, jetsonPort, nanoIP, nanoPort);    
	running = true;
    this->speed = speed; //probably going to want more ways to change the speed...
    locationInst.startGPS();
    
    //starts sending the speeds here
    std::thread speedThread(&DriveMode::sendSpeed, this);
    speedThread.detach();
}

//Sends speed in a separate thread
void DriveMode::sendSpeed() 
{
    while(running) 
    {
	    //send wheel speeds
        speedString = out->controlToStr(round(leftWheelSpeed), round(rightWheelSpeed), 0,0);
        out->sendMessage(&speedString);
	    std::this_thread::sleep_for(std::chrono::milliseconds(50)); //waits 50ms before sending the speed again
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

    double kp = .5, ki = .00005;
    errorAccumulation += error * time;
    PIDValues[0] = speed - (error * kp + errorAccumulation * ki);
    PIDValues[1] = speed + (error * kp + errorAccumulation * ki);

    int max = speed + 40; //forces it to arc when driving
    int min = speed - 40;
    if(PIDValues[0] > max) PIDValues[0] = max;
    if(PIDValues[1] > max) PIDValues[1] = max;
    if(PIDValues[0] < min) PIDValues[0] = min;
    if(PIDValues[1] < min) PIDValues[1] = min;
    return PIDValues;
}

void DriveMode::printSpeeds()
{
    std::cout << "Left Wheels: " << round(leftWheelSpeed) << std::endl;
    std::cout << "Right Wheels: " << round(rightWheelSpeed) << std::endl;
}

bool DriveMode::driveAlongCoordinates(std::vector<std::vector<double>> locations, int id) //used for legs 1-3
{    
    locationInst.startGPSThread();

    std::cout<<"Waiting for GPS connection..." << std::endl;
    while(locationInst.allZero); //waits for the GPS to pick something up before starting
    std::cout << "Connected to GPS" << std::endl; 
     
    //Drives for 4 seconds to hopefully get a good angle from the gps
    leftWheelSpeed = speed;
    rightWheelSpeed = speed;
    printSpeeds();
    cv::waitKey(4000);
    
    float bearingTo;
    std::vector<double> wheelSpeeds;
    for(int i = 0; i < locations.size(); ++i)
    {
        time = 0;
        errorAccumulation = 0;
        while(locationInst.distanceTo(locations[i][0], locations[i][1]) > 0.001) //.001km
        {
            bearingTo = locationInst.bearingTo(locations[i][0], locations[i][1]);
            wheelSpeeds = getWheelSpeeds(bearingTo, speed);
            leftWheelSpeed = wheelSpeeds[1];
            rightWheelSpeed = wheelSpeeds[0];
            printSpeeds();            

            cv::waitKey(100); //waits for 100ms
            time += 100; //updates time
            
            if(tracker.findAR(id))
            {
                locationInst.stopGPSThread();
                std::cout << "Found tag!!!" << std::endl;
                return true;
            }
        }
        if(tracker.findAR(id))
        {
            locationInst.stopGPSThread();
            std::cout << "Found tag!!!" << std::endl;
            return true;
        }
    }
    locationInst.stopGPSThread();
    std::cout << "Made it to the gps location without seeing the tag..." << std::endl;
    return false; //got to gps location without finding the wanted ar tag
}

bool DriveMode::trackARTag(int id) //used for legs 1-3
{
    std::string str;
    std::vector<double> wheelSpeeds;
    int timesNotFound = -1;
    int stopDistance = 250;  //drives until the distance to the tag is less than stopDistance in cm. NOTE: rover only needs to be within 300cm to score.
    
    tracker.trackAR(id); //gets an intial angle from the main camera
    
    //turns to face the artag directly before driving to it. May want to convert to PID although this also shouldn't have to be super accurate.
    while(tracker.angleToAR > 30 || tracker.angleToAR < -25 || tracker.angleToAR == 0) //its 0 if it doesn't see it, camera is closer to the left which is why one is 10 and the other is -5
    {
        if(tracker.trackAR(id))
        {    
            /*if(timesNotFound == -1)
            {
                stops for a second to let things settle
                leftWheelSpeed = 0;
                rightWheelSpeed = 0;
                cv::wait(1000);
            } */       
            wheelSpeeds = getWheelSpeeds(tracker.angleToAR, 0); //pivot turn with pid. May need to multiply this by a constant
            leftWheelSpeed = wheelSpeeds[1] * 2;
            rightWheelSpeed = wheelSpeeds[0] * 2;
            std::cout << tracker.angleToAR << " " << tracker.distanceToAR << std::endl;
            timesNotFound = 0;
        }
        else if(timesNotFound == -1)// hasn't seen anything yet so turns to the left until it sees it
        {
            leftWheelSpeed = -45;
            rightWheelSpeed = 45;
            std::cout << "Haven't seen it so turning left" << std::endl;
        }
        else if(timesNotFound < 10)
        {
            timesNotFound++;
            std::cout << "Didn't find it " << timesNotFound << " times" << std::endl;
        }
        else
        {
            leftWheelSpeed = 0;
            rightWheelSpeed = 0;
            std::cout << "we lost it..." << std::endl;
            return false; //TODO: do something about this
        }
        cv::waitKey(100);
    }
    
    time = 0;
    errorAccumulation = 0; 
    std::cout << "We are locked on and ready to track!" << std::endl;
    
    while(tracker.distanceToAR > stopDistance || tracker.distanceToAR == -1) //distance = -1 if the camera cannot find a tag
    {
        if(tracker.trackAR(id) || timesNotFound < 10)
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
            leftWheelSpeed = wheelSpeeds[1];
            rightWheelSpeed = wheelSpeeds[0];
            printSpeeds();
        }
        else
        {
            //Stops the rover
            leftWheelSpeed = 0;
            rightWheelSpeed = 0;
            std::cout << "Tag not found" << std::endl;
            return false; //TODO: do something about this
        }
        cv::waitKey(100); //waits for 100ms    
        time += 100;
    }
    leftWheelSpeed = 0;
    rightWheelSpeed = 0;
    return true;
}
