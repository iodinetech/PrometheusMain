/* This file is part of Prometheus.
*/
/** @author 郑聪
 *  @date 2020/04/20
 */
#ifndef _KADEMLIAHOST_H_
#define _KADEMLIAHOST_H_

#include <kademlia/kademlia.pb.h>

#include "h256.h"
#include "common.h"
#include "nodeTable.h"
#include "session.h"
#include "tools/BlockingQueue.h"
#include "net/MessageDispacher.h"
#include <net/Connector.h>
#include <tools/Logger.h>
#include <boost/asio.hpp>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <memory.h>
namespace Prometheus
{



/** @brief 负责处理kademlia网络的事务的类
 * kademlia算法主要的部分就在这里。
 * 这个类负责主动发起以下的四个操作：
 * Ping(),FindNode()，同时可以自己发现更多的peer
 * 及处理接收的来自外部节点的FindValue()和FindNode()的回复。
 */
class KademliaHost
{
public:
    KademliaHost(const NodeID& _mynodeid,ba::io_service& iosvc,
    BlockingQueue<UDPPacket>& _outqueue,BlockingQueue<UDPPacket>& _inqueue,
    NodeTable& _peerTable,Logger& _logger, bi::tcp::endpoint localep):
    MyNodeID(_mynodeid),_iosvc(iosvc),TCPconn(iosvc,bi::tcp::endpoint(bi::tcp::v4(),localep.port())),outQueue(_outqueue),inQueue(_inqueue),
    dispacher(std::bind(&KademliaHost::defaultDispacherCallback,this,std::placeholders::_1,std::placeholders::_2)),logger(_logger),
    onPingingCallback(std::bind(&KademliaHost::onPinging,this,std::placeholders::_1,std::placeholders::_2))
    {
        m_timer.reset(new boost::asio::deadline_timer(iosvc));
        peerTable = std::make_shared<NodeTable>(_peerTable);
        dispacher.registerCallback(onPingingCallback);
        
        std::function<void(const std::shared_ptr<PingReplyPacket>&,const boost::asio::ip::udp::endpoint&)> onPingingReplyCallback
        (std::bind(&KademliaHost::onPingingReply,this,std::placeholders::_1,std::placeholders::_2));
        dispacher.registerCallback(onPingingReplyCallback);
        
        std::function<void(const std::shared_ptr<FindNodePacket>&,const boost::asio::ip::udp::endpoint&)> onFindNodeCallback
        (std::bind(&KademliaHost::onFindNode,this,std::placeholders::_1,std::placeholders::_2));
        
        std::function<void(const std::shared_ptr<FindNodeReplyPacket>&,const boost::asio::ip::udp::endpoint&)> onFindNodeReplyCallback
        (std::bind(&KademliaHost::onFindNodeReply,this,std::placeholders::_1,std::placeholders::_2));
        dispacher.registerCallback(onFindNodeReplyCallback);
        ReceiveFile();
    }

    //向列表中的一个节点发出ping包，来探测那个节点是否在线
    void Ping(std::shared_ptr<NodeInformation> dest);

    //将一个值存放在网络中
    void Store(const ValueID& Key,const bytes Value);

    //查询网络中是否存在NodeID为node的节点，并接收最靠近被查找节点的K个节点的三元组
    void FindNode(std::shared_ptr<NodeInformation> node); 

    //处理外部节点发来的节点查询请求
    void onFindNode(const std::shared_ptr<FindNodePacket>& fnp, const bi::udp::endpoint& ep);

    //处理外部节点发来的FindNodePacket的回复
    void onFindNodeReply(const std::shared_ptr<FindNodeReplyPacket>& fnrp, const bi::udp::endpoint& ep);

    //处理外部节点发来的PingPacket的回复
    void onPinging(const std::shared_ptr<PingPacket>& pp,const bi::udp::endpoint& ep);

    //处理外部节点发来的PingReplyPacket的回复
    void onPingingReply(const std::shared_ptr<PingReplyPacket>& prp,const bi::udp::endpoint& ep);

    void ReceiveFile();

    bool SendFile(const std::shared_ptr<NodeInformation> node, const std::string& filepath);

    std::shared_ptr<NodeInformation> LookUpNodeInformation(const NodeID &id)
    {
        return peerTable->LookUpNodeInformation(id);
    }

    void run()
    {
        while(true)
        {
            for(auto i:AlivePeers)
                Ping(i);
            sleep(15);
        }
    }

    void muxer()
    {
        auto p = inQueue.take();
        dispacher.onMessage(p.content,p._endpoint);
    }

    void defaultDispacherCallback(const std::shared_ptr<google::protobuf::Message>& message,const boost::asio::ip::udp::endpoint& ep)
    {
        std::cout<<"not Kademlia Message.TODO:Forwarding to upper layer."<<std::endl;
    }

    //添加新节点，并尝试ping这个节点，判断是否存活。
    void addNode(std::shared_ptr<NodeInformation> newNode, bool isActive = true);
private:

    //添加定时任务
    void schedule(unsigned interval, std::function<void(boost::system::error_code const&)> f);
    
    //定时任务，检查是否有等待被驱逐而且计时器到期的节点，如果有则清除。
    void checkEvictions();
    void acceptHandler(std::shared_ptr<Session> session,boost::system::error_code e);
    void headerHandler(boost::system::error_code e,size_t len);
    void contentHandler(boost::system::error_code e,size_t len);
    NodeID MyNodeID; //本机的NodeID
    ba::io_service& _iosvc;
    bi::tcp::acceptor TCPconn;
    //bi::tcp::socket TCPSocket;
    BlockingQueue<UDPPacket>& outQueue;
    BlockingQueue<UDPPacket>& inQueue;

    std::shared_ptr<NodeTable> peerTable; //节点列表，处理网络中节点信息CRUD的工作

    MessageDispacher dispacher; 

    Logger logger;

    std::unique_ptr<boost::asio::deadline_timer> m_timer;
    std::set<std::shared_ptr<NodeInformation>> AlivePeers;
    //记录了最长时间未联系节点被ping的时间，用来计算超时
    std::map<std::shared_ptr<NodeInformation>,std::chrono::steady_clock::time_point> NodeToBeEvicted;

    //记录了向这些节点发送findNode消息的时间
    std::map<std::shared_ptr<NodeInformation>,std::chrono::steady_clock::time_point> FindingNode;

    std::vector<std::shared_ptr<NodeInformation>> FindNodeReply;
    std::map<std::shared_ptr<NodeInformation>,bool> FindNodeVisited;
    std::set<std::shared_ptr<NodeInformation>> PendingNodes;

    std::chrono::milliseconds const EvictionInterval = std::chrono::milliseconds(75);
    std::chrono::milliseconds const FindNodeInterval = std::chrono::milliseconds(300);
    std::chrono::milliseconds const PendingInterval = std::chrono::milliseconds(300);

    const unsigned int c_keepAliveInterval = 1000;
    std::function<void(const std::shared_ptr<PingPacket>&,const boost::asio::ip::udp::endpoint&)> onPingingCallback;
    std::vector<char> sendbuffer;
    std::vector<std::unique_ptr<boost::asio::deadline_timer>> timers;
    //记录收到的FindNode的数量
    unsigned int FindNodeReceiveCount;
    std::set<std::shared_ptr<NodeInformation>> triedNodes; //FindNode请求中已经发送过请求的节点
    unsigned CurrentFindNodeRound = 0;
    unsigned MaxFindNodeRound = 5; //FindNode请求的最大轮数
    int alpha = 3; // findNode时发送消息的节点的数量

    FILE *fp; //被传送的文件
};



} // namespace Prometheus

#endif