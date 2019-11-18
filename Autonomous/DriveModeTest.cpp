#include <vector>
#include <iostream>
#include "DriveMode.h"

using namespace std;

int main()
{
    
   DriveMode testInst("/dev/video0", /*speed*/1.0);
   vector<vector<double>> locations;
   vector<double> locationArray1;//[2] = {1, 1};
   locationArray1.push_back(1.0);
   locationArray1.push_back(1.0);
   locations.push_back(locationArray1);
  
   
   testInst.driveAlongCoordinates(locations, 3);
   testInst.driveAlongCoordinates(locations, 9999);
   testInst.driveAlongCoordinates(locations, -1);
   
   testInst.trackARTag(5);
   testInst.trackARTag(5000);
   testInst.trackARTag(-10);
   testInst.trackARTag(0);




    return 0;
}
