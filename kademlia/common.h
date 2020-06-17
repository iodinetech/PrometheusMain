/* This file is part of Prometheus.
*/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <list>
#include <array>
#include <boost/asio.hpp>
#include <vector>

namespace Prometheus
{

typedef unsigned char byte;
using NodeID = std::array<unsigned char,32>;
typedef std::array<unsigned char,32> ValueID;
typedef std::vector<char> bytes;
namespace bi = boost::asio::ip;
namespace ba = boost::asio;
const int NodeIDSize = 32;
/** @brief 存储节点ID、IP地址、UDP端口的三元组
 */
struct NodeInformation
{
    NodeInformation(NodeID& _nodeID,const boost::asio::ip::address& _NodeAddress,const unsigned short _UDPPort):
    nodeID(_nodeID),NodeAddress(_NodeAddress),UDPPort(_UDPPort)
    {}
    NodeInformation(){}
    NodeID nodeID; /**< 节点ID */
    boost::asio::ip::address NodeAddress; /**< 节点的IP地址 */
    uint16_t UDPPort; /**< 节点的UDP端口 */
    bool Visited; /**< 在进行findNode请求时需要寻找未在之前几轮访问过的节点 */
    bool Pending; /**< 被ping但是未回复 */
};

struct File_info {    
	typedef unsigned long long Size_type;    
	Size_type filesize;    
	size_t filename_size;    
	File_info() : filesize(0), filename_size(0) {}  
};

    
} // namespace Prometheus

#endif 