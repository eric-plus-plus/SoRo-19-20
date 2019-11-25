# Autonomous
Everything needs to be run using make files. Simple g++ with not work without linking.

## How to Compile and Run:
### Install Dependencies:
Install OpenCV following https://askubuntu.com/questions/1123955/install-opencv4-in-ubuntu-16 <br>
Install Aruco following http://miloq.blogspot.com/2012/12/install-aruco-ubuntu-linux.html <br>

### Build the Project
Navigate to SoRo-19-20/Autonomous and run the following code: <br>
mkdir build && cd build <br>
cmake .. <br>
make #this line must be run anytime code is changed to recompile <br>

### Running:
After building successfully, run <br>
./autonomous
