# Mission Control

This is all the code for either programs running on the MC computers or the rover's computer.

## udp_mc_drive

This is the drive remote control code from Soro4. It is programmed in Qt. The config file needs to know the 
ip of the computer it is running on (port doesn't really matter here) and the ip and port of the drive 
controller arduino. (10.0.0.1:1001)

## udp_mc_arm

This is the arm remote control code for the linear actuator arm. Ideally, this should be updated. It made with Qt, and in it's current state, is made to run on the local IP address (127.0.0.1:1237). It communicates with the udp_rover program over ethernet.

## udp_rover

This was the microcontroller communication solution before the Aurdinos had ethernet shields. This program recieved UDP datagrams and sent them out to the correct device over serial depending on an ID. It was also called the "UDP Hub" and ran on the rover's computer, but you can run it locally for testing.

Right now it is configured to run on the local IP address (127.0.0.1:1237)
