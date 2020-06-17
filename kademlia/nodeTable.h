/* This file is part of Prometheus.
*/
#ifndef _NODETABLE_H_
#define _NODETABLE_H_

#include "common.h"
#include <list>
#include <array>
#include <vector>
#include <chrono>
#include "tools/Logger.h"
#include "tools.h"

namespace Prometheus
{

/** @brief 管理k-bucket的类。主要进行k-bucket的增改删查，以及定时ping操作的通知。
 */ 
class NodeTable {
public:
    NodeTable(NodeID& _MyNodeID,Logger _logger):
    MyNodeID(_MyNodeID),logger(_logger){}

    //查询XOR距离最接近输入节点的K个节点。返回一个vector<NodeInformation>。
    std::vector<std::shared_ptr<NodeInformation>> LookUpKNearestNodes(const NodeID &id);

    //查询k-bucket中一个节点的信息。
    std::shared_ptr<NodeInformation> LookUpNodeInformation(const NodeID &id);

    //向Kbucket中添加一个新节点。
    void addNode(std::shared_ptr<NodeInformation> newNode);

    void deleteNode(std::shared_ptr<NodeInformation> node)
    {
        auto& bucket = kBucket[xordistance(node->nodeID,MyNodeID)];
        for(auto i = bucket.begin();i != bucket.end();i++)
            if(*i == node)
            {
                i=bucket.erase(i);
            }
    }
    bool isBucketFull(const NodeID& id);

    //查询一个NodeID对应的bucket中最久未联系节点
    std::shared_ptr<NodeInformation> getLeastRecentNode(const NodeID& id)
    {
        return kBucket[xordistance(id,MyNodeID)].front();
    }

    //仅供调试使用，不可在别的地方调用
    const std::array<std::vector<std::shared_ptr<NodeInformation>>,257>& getBucket(){return kBucket; }
private:
    //删除Kbucket中的一个节点。
    void deleteNode(NodeID node);

    //定时任务，检查待驱逐节点回复是否超时。如果超时则删除。
    void checkNodeToBeEvicted();
    
    //存储节点信息的桶，每个桶中的元素离本机的异或距离相同。
    std::array<std::vector<std::shared_ptr<NodeInformation>>,257> kBucket;

    uint32_t NumBuckets = 256;

    uint32_t MaxBucketSize=3;
    uint32_t PropagationCoefficient = 3; // 论文中执行递归寻址操作时每次请求的节点数量。

    uint32_t k = 3; //论文中每次寻找节点操作返回的节点数量。

    NodeID MyNodeID;
    Logger logger;

};
} //namespace Prometheus

#endif