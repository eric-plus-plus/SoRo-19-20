// Eric Rackelin
// 11/24/2019

// This is a class to make sending messages to Arduinos easier

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>

class UDPOut
{
    public:
        UDPOut(const char* host_ip, int host_port, const char* dest_ip, int dest_port);
        ~UDPOut();
        void sendMessage(std::string* str); //sends the string returned from the two methods below
        std::string controlToStr(int leftWheels, int rightWheels, int gimbalTilt, int gimbalPan); //gets string to control the wheels and gimble when its implimented
        std::string ledToStr(bool red, bool green, bool blue); //gets string to change the color of the leds
    
    private:
        char* myIp;
        int myPort;
        char* destIp;
        int destPort;
        
        int sockfd;
        struct sockaddr_in myaddr, destaddr;
};
