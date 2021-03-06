#include "UDPOut.h"

UDPOut::UDPOut(const char* host_ip, int host_port, const char* dest_ip, int dest_port)
{   
    // create socket file descriptor
    // c is nasty
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // setup this socket's address
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    inet_pton(AF_INET, host_ip, &(myaddr.sin_addr));
    myaddr.sin_port = htons(host_port);
    
    // setup destination socket's address
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip, &(destaddr.sin_addr));
    destaddr.sin_port = htons(dest_port);
    
    // bind the socket
    int error = bind(sockfd, (const struct sockaddr *)&myaddr, sizeof(myaddr));
    if (error < 0)
    {
        printf("error with socket\n");
    }
}

std::string UDPOut::controlToStr(int leftWheels, int rightWheels, int gimbalTilt, int gimbalPan)
{
   // printf("TEMP left: %d right: %d\n", leftWheels, rightWheels);
    std::string str = "";
    str += char(-127);
    str += char(0);
    str += char(0);
    str += char(leftWheels);
    str += char(rightWheels);
    str += char(gimbalTilt);
    str += char(gimbalPan);
    str += char( (leftWheels + rightWheels + gimbalTilt + gimbalPan)/5 );
    
    //printf("%d\n", str.length());
    
    return str;
}

std::string UDPOut::ledToStr(bool red, bool green, bool blue)
{
    std::string str = "";
    str += char(-127);
    str += char(1);
    char temp = 0;
    if (red)
        temp |= (1 << 2);
    if (green)
        temp |= (1 << 1);
    if (blue)
        temp |= (1 << 0);
    str += temp;
    str += temp;
    return str;
}

UDPOut::~UDPOut()
{
    shutdown(sockfd, 2);
    close(sockfd);
}

void UDPOut::sendMessage(std::string* str)
{
    const char* cstr = (*str).c_str();
    int len = str->length();
    //printf("Sending: %s\n", cstr); //this will not print characters the terminal can print...
    
    sendto(sockfd, (const char *)cstr, len, 0, (const struct sockaddr *) &destaddr, sizeof(destaddr));
}
