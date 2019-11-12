#include <vector>
#include <iostream>
#include "DriveMode.h"

using namespace std;

int main()
{
    
   DriveMode testInst;
   vector<double> locations;
   int[] locationArray1 = {1, 1};
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
