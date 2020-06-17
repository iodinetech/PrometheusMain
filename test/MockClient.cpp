#include <boost/asio.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;

int main()
{
    ip::udp::endpoint ep(ip::address::from_string("127.0.0.1"),1234);
    io_service iosvc;
    ip::udp::socket s(iosvc);
    s.connect(ep);
    string str = "test";
    s.send_to(buffer(str),ep);
    return 0;
}