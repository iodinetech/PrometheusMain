#include "net/MessageDispacher.h"
#include "kademlia/kademlia.pb.h"
#include <functional>
#include "kademlia/h256.h"
#include <memory>
#include <vector>
#include <iostream>
using namespace Prometheus;
using namespace std;
int main()
{
    function<void(const shared_ptr<google::protobuf::Message>&)> deff = [](const shared_ptr<google::protobuf::Message> &msg){cout<<"default message"<<endl;};

    function<void(const shared_ptr<PingPacket>&)> f = [](const shared_ptr<PingPacket>& _p){cout<<_p->random();};

    MessageDispacher md(deff);

    PingPacket p;
    Randomizer r;
    p.set_random(r.getRandomInt32());
    cout<<"random="<<p.random()<<endl;

    vector<char> buffer;

    md.makeMessage(p,buffer);

    md.registerCallback<PingPacket>(f);

    md.onMessage(buffer);

}