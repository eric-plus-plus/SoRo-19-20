# Autonomous
Everything needs to be run using make files. Simple g++ with not work without linking.

## How to Compile and Run:
### Install Dependencies:
Install OpenCV following https://askubuntu.com/questions/1123955/install-opencv4-in-ubuntu-16 <br>
Install Aruco following http://miloq.blogspot.com/2012/12/install-aruco-ubuntu-linux.html <br>

### Build the Project:
Navigate to SoRo-19-20/Autonomous and run the following code: <br>
mkdir build && cd build <br>
cmake .. <br>
make 

### Running:
After building successfully, go back to SoRo-19-20/Autonomous and run <br>
bash runAutonomous.sh

### Notes:
Run v4l2-ctl --list-devices to find which port each camera is plugged into when editting config <br>
To change file that is executed, change the executable specified in CMakeLists.txt and rerun cmake .. <br>
To recompile the code, rerun "make" in build <br>
To clean, delete build and rerun the commands in the "Build the Project" section
