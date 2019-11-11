#include <vector>
#include <iostream>
#include "DriveMode.h"

int main()
{
    
   DriveMode testInst;
   vector<double> locations;
   locations.push_back({1,1});
   locations.push_back({400, 30});
   locations.push_back({0, 5});
   locations.push_back({-20, 7});
   
   testInst.driveAlongCoordinates(locations, 3);
   
   testInst.trackARTag(5);
   testInst.trackARTag(5000);
   testInst.trackARTag(-10);
   testInst.trackARTag(0);




    return 0;
}
