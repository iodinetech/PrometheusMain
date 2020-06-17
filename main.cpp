#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <functional>
#include "kademlia/h256.h"
#include "tools/ThreadPool.h"
#include "kademlia/kademliaHost.h"
#include "tools/BlockingQueue.h"
#include "net/Connector.h"
#include "console/server.h"
#include <unistd.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace Prometheus;
using namespace std;
using namespace boost::asio;
using namespace boost::property_tree;

int main(int argc, char *argv[])
{
    boost::property_tree::ptree root;
    boost::property_tree::read_json<boost::property_tree::ptree>("config.json",root);
    string strmyid = root.get<string>("id");
    string myip = root.get<string>("ip");
    string myport = root.get<string>("port");
    uint16_t UDPPort = atoi(myport.c_str());

    NodeID myid = hex2char(strmyid);

    ip::udp::endpoint udpLocalEp(ip::udp::v4(),UDPPort);
    ip::tcp::endpoint tcpLocalEp(ip::tcp::v4(),UDPPort);
    io_service iosvc;
    io_service::work w(iosvc);
    BlockingQueue<UDPPacket> inq;
    BlockingQueue<UDPPacket> outq;
    Logger logger("host");
    logger.SetLevel(LogLevel::debug);
    Prometheus::Randomizer r;
    NodeID nid = myid;
    NodeTable nt(nid,logger);
    shared_ptr<UDPConnector> uc(new UDPConnector(outq,inq,iosvc,logger,udpLocalEp));
    uc->connect();
    shared_ptr<KademliaHost> kh(new KademliaHost(nid,iosvc,outq,inq,nt,logger,tcpLocalEp));
    shared_ptr<rpcHost> rh(new rpcHost(kh,logger));
    uc->registerCallback(std::bind(&KademliaHost::muxer,kh.get()));
    ThreadPool tp("host");
    tp.start(4);
    tp.run(std::bind(&KademliaHost::run,kh.get()));
    tp.run(std::bind(&rpcHost::run,rh.get()));
    tp.run(std::bind(&UDPConnector::run,uc.get()));
    tp.run(std::bind(&UDPConnector::recv,uc.get()));
    ptree network;
    ptree nodes;
    ptree nodeList;
    read_json<ptree>("network.json",network);
    nodes=network.get_child("nodes");
    vector<shared_ptr<NodeInformation>> idlist;
    string inputidstr,inputaddr,inputport;
    for(ptree::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        nodeList = it->second;
        inputidstr=nodeList.get<string>("id");
        inputaddr=nodeList.get<string>("ip");
        inputport=nodeList.get<string>("port");
        NodeID inputid = hex2char(inputidstr);
        shared_ptr<NodeInformation> pNode(new NodeInformation(inputid,bi::address::from_string(inputaddr),atoi(inputport.data())));
        idlist.emplace_back(pNode);
    }
    for(auto i: idlist)
    {
        kh->addNode(i);
        
    }
    iosvc.run();
}