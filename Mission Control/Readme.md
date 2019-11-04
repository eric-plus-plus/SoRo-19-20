# Mission Control

This is all the code for either programs running on the MC computers or the rover's webserver.

The program udp_mc_drive is the drive code from Soro4. It is programmed in Qt. The config file needs to know the 
ip of the computer it is running on (port doesn't really matter here) and the ip and port of the drive 
controller arduino. (10.0.0.1:1001)
