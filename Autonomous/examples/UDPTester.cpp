#include <libs/UDPOut.h>

int main()
{
    UDPOut* out = new UDPOut("192.168.1.100", 1256, "192.168.1.106", 1001);
    
    std::string str = controlToStr(90, 90, 0, 0);
    
    out->sendMessage(&str);
    
    delete out;
    
    return 0;
}
