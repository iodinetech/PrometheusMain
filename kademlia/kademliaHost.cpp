#include "kademliaHost.h"
#include "kademlia/kademlia.pb.h"
#include "net/Connector.h"
#include "tools.h"
#include "common.h"
#include "session.h"
#include <memory>
#include <chrono>
#include <vector>
#include <boost/asio.hpp>

using namespace std;
namespace Prometheus
{

/** 
 * @brief 向一个指定的IP地址发送Kademlia的PingPacket，只有一个节点要被踢出列表时才会使用
 * @param addr 指定的IP地址
 * @param port 指定的端口
 */
void KademliaHost::Ping(shared_ptr<NodeInformation> dest)
{
    AlivePeers.emplace(dest);
    logger.Log(LogLevel::debug,"pinging "+dest->NodeAddress.to_string()+":"+to_string(dest->UDPPort));
    PingPacket p;
    Randomizer r;
    p.set_random(r.getRandomInt32());
    if(shared_ptr<NodeInformation> item = peerTable->LookUpNodeInformation(dest->nodeID); item == nullptr)
    {
        dest->Pending = true;
        PendingNodes.insert(dest);
    }
    p.set_nodeid(reinterpret_cast<char*>(MyNodeID.data()));
    sendbuffer.reserve(512);
    sendbuffer.clear();
    dispacher.makeMessage(p,sendbuffer);
    bi::udp::endpoint ep(dest->NodeAddress,dest->UDPPort);
    UDPPacket up(sendbuffer,ep);
    outQueue.put(up);
}
/**
 * @brief 收到PingPacket时的处理函数。
 * 
 * @param pp 收到的PingPacket
 * @param ep 发送方的端点
 */
void KademliaHost::onPinging(const shared_ptr<PingPacket>& pp,const bi::udp::endpoint& ep)
{
    string id = char2hex(reinterpret_cast<const unsigned char*>(pp->nodeid().data()),32);
    logger.Log(LogLevel::debug,"ping packet from"+id+" "+ep.address().to_string()+":"+to_string(ep.port()));
    //将对方添加进自己的节点列表
    NodeID newNodeID = hex2char(id);
    shared_ptr<NodeInformation> newnode(new NodeInformation(newNodeID,ep.address(),ep.port()));
    addNode(newnode,false);
    logger.Log(LogLevel::debug,"sending pingreplypacket to "+ep.address().to_string()+":"+to_string(ep.port())+"\n");
    PingReplyPacket p;
    p.set_random(pp->random());
    p.set_nodeid(reinterpret_cast<char*>(MyNodeID.data()));
    sendbuffer.reserve(512);
    sendbuffer.clear();
    dispacher.makeMessage(p,sendbuffer);
    UDPPacket up(sendbuffer,ep);
    outQueue.put(up);

}

void KademliaHost::onPingingReply(const shared_ptr<PingReplyPacket>& prp, const bi::udp::endpoint& ep)
{
    string id = char2hex(reinterpret_cast<const unsigned char*>(prp->nodeid().data()),32);
    logger.Log(LogLevel::debug,"pingreply packet from "+id+" "+ep.address().to_string()+":"+to_string(ep.port()));
    NodeID inconmingID;
    auto temp = prp->nodeid();
    for(int j = 0;j<32;j++)
        inconmingID[j] = temp[j];
    //判断节点是否处于待驱逐列表中
    auto iter = NodeToBeEvicted.find(peerTable->LookUpNodeInformation(inconmingID));
    if(iter != NodeToBeEvicted.end())
    {
        NodeToBeEvicted.erase(iter);
        peerTable->addNode(iter->first);
    }
    //判断节点是否属于新添加的，还未确认连通的节点列表
    for(auto i = PendingNodes.begin();i != PendingNodes.end(); i++)
        if((*i)->nodeID == inconmingID)
        {
            (*i)->Pending = false;
            peerTable->addNode(*i);
            i = PendingNodes.erase(i);
            if(PendingNodes.empty() || i == PendingNodes.end())break;
        }
    
}

//此函数只能在第一轮时调用！
void KademliaHost::FindNode(shared_ptr<NodeInformation> node)
{
    auto SendSet = peerTable->LookUpKNearestNodes(node->nodeID);
    Randomizer r;
    for(auto i:SendSet)
    {
        triedNodes.insert(i);
        FindNodePacket p;
        p.set_nodeid(reinterpret_cast<char*>(i->nodeID.data()));
        p.set_random(r.getRandomInt32());
        FindingNode[i]=chrono::steady_clock::now();
        vector<char> buffer;
        buffer.reserve(512);
        dispacher.makeMessage(p,buffer);
        bi::udp::endpoint ep(i->NodeAddress,i->UDPPort);
        UDPPacket packet(buffer,ep);
        outQueue.put(packet);
    } 
}

void KademliaHost::onFindNode(const shared_ptr<FindNodePacket>& prp, const bi::udp::endpoint& ep)
{
    string id = prp->nodeid().data();
    NodeID nid;
    Randomizer r;
    for(int i = 0;i<32;i++)
        nid[i]=id[i];
    vector<shared_ptr<NodeInformation>> result = peerTable->LookUpKNearestNodes(nid);
    FindNodeReplyPacket p;
    for(auto i:result)
    {
        string tempid = reinterpret_cast<const char*>(i->nodeID.data());
        NodeInfo* nodeinfo = p.add_nodelist();
        nodeinfo->set_ip(ep.address().to_string());
        nodeinfo->set_random(r.getRandomInt32());
        nodeinfo->set_port(ep.port());
        nodeinfo->set_nodeid(reinterpret_cast<const char*>(i->nodeID.data()));
    }
    p.set_random(r.getRandomInt32());
    vector<char> buffer;
    buffer.reserve(512);
    dispacher.makeMessage(p,buffer);
    UDPPacket packet(buffer,ep);
    outQueue.put(packet);
}

void KademliaHost::onFindNodeReply(const shared_ptr<FindNodeReplyPacket>& fnrp, const bi::udp::endpoint& ep)
{
    //todo:检查这个包的目标节点是不是我们想要的目标节点
    Randomizer r;
    auto nodes = fnrp->nodelist();
    NodeID targetid = hex2char(fnrp->target().nodeid());
    for(auto i: nodes)
    {
        NodeID id = hex2char(i.nodeid().data());
        shared_ptr<NodeInformation> newnode(new NodeInformation(id,bi::address::from_string(i.ip()),i.port()));
        addNode(newnode,true);
    }
    CurrentFindNodeRound++;
    if(CurrentFindNodeRound < MaxFindNodeRound)
    {
        int i=0;
        auto nearests =peerTable->LookUpKNearestNodes(targetid);
        while(i<alpha)
        {
            if(!triedNodes.count(nearests[i]))
            {
                FindNodePacket p;
                p.set_nodeid(char2hex(nearests[i]->nodeID.data(),NodeIDSize));
                p.set_random(r.getRandomInt32());
                bytes buffer;
                dispacher.makeMessage(p,buffer);
                bi::udp::endpoint ep(nearests[i]->NodeAddress,nearests[i]->UDPPort);
                UDPPacket packet(buffer,ep);
                outQueue.put(packet);
            }
            triedNodes.insert(nearests[i]);
        }
    }
    else
    {
        CurrentFindNodeRound = 0;
        triedNodes.clear();
    }
}

void KademliaHost::checkEvictions()
{
    schedule(EvictionInterval.count(),[this](boost::system::error_code ec){
        vector<shared_ptr<NodeInformation>> onEviction;
        unsigned remainingToEvict = 0;
        for(auto i: NodeToBeEvicted)
        {
            if(chrono::steady_clock::now()-i.second > PendingInterval)
            {
                onEviction.push_back(i.first);
            }
        }
        for(auto i:onEviction)
        {
            peerTable->deleteNode(i);
            AlivePeers.erase(i);
        }
        remainingToEvict = NodeToBeEvicted.size()-onEviction.size();
        if(remainingToEvict)
            checkEvictions();
    });
}

void KademliaHost::addNode(std::shared_ptr<NodeInformation> newNode, bool isActive)
{
    //如果已经加入过，那么就只要将它移动到最后面.
    auto& bucket = peerTable->getBucket();
    if(peerTable->LookUpNodeInformation(newNode->nodeID) != nullptr)
    {
        peerTable->addNode(newNode);
    }
    else
    {
        logger.Log(LogLevel::debug,"new node!");

        //否则就要加入了
        bool isFull = peerTable->isBucketFull(newNode->nodeID);
        if(isFull)
        {
            logger.Log(LogLevel::debug,"bucket full!");
            shared_ptr<NodeInformation> leastRecent = peerTable->getLeastRecentNode(newNode->nodeID);
            Ping(leastRecent);
            NodeToBeEvicted[leastRecent] = chrono::steady_clock::now();
            int evictSize = NodeToBeEvicted.size();
            if(evictSize == 1)
                checkEvictions();
        }
        else{
            newNode->Pending=true;
            //peerTable->addNode(newNode);
            if(isActive) //如果是传入的包来自新节点，就不需要主动ping了
                Ping(newNode);
            else
                peerTable->addNode(newNode);
                
        } 
    }
}

void KademliaHost::schedule(unsigned interval, std::function<void(boost::system::error_code const&)> f)
{
    unique_ptr<ba::deadline_timer> t(new ba::deadline_timer(_iosvc));
    t->expires_from_now(boost::posix_time::milliseconds(interval));
    t->async_wait(f);
    timers.emplace_back(move(t));
}


void KademliaHost::ReceiveFile()
{
    shared_ptr<Session> session = Session::create(TCPconn.get_io_service());
    TCPconn.async_accept(session->socket(),std::bind(&KademliaHost::acceptHandler,this,session,std::placeholders::_1));
}

void KademliaHost::acceptHandler(shared_ptr<Session> session,boost::system::error_code e)
{
    session->start();
    ReceiveFile();
}

bool KademliaHost::SendFile(const shared_ptr<NodeInformation> targetnode, const string& filename)
{
  
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == NULL) {
        std::cerr << "cannot open file\n";
        return false;
    }
    boost::shared_ptr<FILE> file_ptr(fp, fclose);
  
    const size_t k_buffer_size = 32 * 1024;
    char buffer[k_buffer_size];
    File_info file_info;
  
    int filename_size  = filename.length() + 1;
    size_t file_info_size = sizeof(file_info);
    size_t total_size = file_info_size + filename_size;
    if (total_size > k_buffer_size) {
        std::cerr << "File name is too long";
        return false;
    }
    file_info.filename_size = filename_size;
  
    fseek(fp, 0, SEEK_END);
    file_info.filesize = ftell(fp);
    rewind(fp);

    memcpy(buffer, &file_info, file_info_size);
    memcpy(buffer + file_info_size, filename.c_str(), filename_size);
    bi::tcp::socket sock(_iosvc);
    sock.connect(bi::tcp::endpoint(targetnode->NodeAddress, targetnode->UDPPort));

    logger.Log(LogLevel::info,"Sending file: "+filename);
    size_t len = total_size;
    unsigned long long total_bytes_read = 0;
    while (true) {
        sock.send(ba::buffer(buffer, len), 0);
        if (feof(fp)) break;
        len = fread(buffer, 1, k_buffer_size, fp);
        total_bytes_read += len;
        logger.Log(LogLevel::info,"percentage: "+to_string(total_bytes_read/total_size));
    }
    return true;

}
} // namespace Prometheus