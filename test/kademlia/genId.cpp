#include <fstream>
#include <iostream>
#include <string>
#include <kademlia/h256.h>
#include <kademlia/tools.h>

using namespace std;
using namespace Prometheus;

int main()
{
    fstream s;
    s.open("Test.txt",ios::out);
    Randomizer r;
    string addr = "127.0.0.1";
    NodeID id = r.getRandomSHA256();
    string strid = char2hex(id.data(),32);
    int16_t port = rand() % 4096 + 1000;
    string strport = to_string(port);
    s<<strid<<" "<<addr<<" "<<strport<<endl;
    for(int i = 0;i<8;i++)
    {
        NodeID newID = id;
        newID[25]^=1<<2;
        newID[30]^=1<<i;
        strid = char2hex(newID.data(),32);
        port = rand() % 4096 + 1000;
        strport = to_string(port);
        s<<strid<<" "<<addr<<" "<<strport<<endl;
    }
    s.close();
    return 0;
}