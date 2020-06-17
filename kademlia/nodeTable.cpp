#include "nodeTable.h"
#include "tools.h"
#include <vector>
#include <iostream>
using namespace std;

namespace Prometheus
{

/**
 * @brief 查找离目标节点最近的k个节点.
 * 
 * @param id 目标节点的id
 * @return vector<shared_ptr<NodeInformation>> 距离目标节点最近的k个节点的信息。
 */

bool NodeTable::isBucketFull(const NodeID& id)
{
    return kBucket[xordistance(id,MyNodeID)].size() >= MaxBucketSize;
}

vector<shared_ptr<NodeInformation>> NodeTable::LookUpKNearestNodes(const NodeID &id)
{
    size_t distance = xordistance(id,MyNodeID);
    cout<<"distance="<<distance<<endl;
    size_t head = distance;
    int last = distance == 0?NumBuckets - 1:(distance - 1)%NumBuckets;
    cout<<"head="<<head<<"last="<<last<<endl;
    int count = k;
    vector<shared_ptr<NodeInformation>> result;
    if(distance > 1 && last != NumBuckets - 1)
    {
        int i = 0;
        while(count>0)
        {
            i=0;
            while(i<kBucket[head].size() && head != NumBuckets - 1)
            {
                result.push_back(kBucket[head][i]);
                i++;
                count--;
                if(count == 0)break;
            }
            if(count>0)
            {
                i=0;
                head++;
                while(i<kBucket[last].size()&& last)
                {
                    result.push_back(kBucket[last][i]);
                    i++;
                    count--;
                    if(count == 0)break;
                }
                if(count>0) last--;
            }
        }
    }
    else if(distance<2)
    {
        cout<<"here!"<<endl;
        int i = 0;
        while(count>0)
        {
            i=0;
            if(head == NumBuckets - 1) break;
            while(i<kBucket[head].size())
            {
                result.push_back(kBucket[head][i]);
                i++;
                count--;
                if(count == 0) break;
            }
            if(count>0) head++;
        }
    }
    else if(last == NumBuckets - 1)
    {
        while(count>0)
        {
            int i = 0;
            if(last == 0) break;
            while(i<kBucket[last].size())
            {
                result.push_back(kBucket[last][i]);
                i++;
                count--;
                if(count == 0)break;
            }
            if(count>0) last--;
        }
    }
    return result;
}

/**
 * @brief 查询一个节点。如果存在，返回该节点的节点信息的智能指针。如果不存在，返回空的智能指针。
 * 
 * @param id 待查询的节点信息
 * @return shared_ptr<NodeInformation> 被查询的节点信息的智能指针。
 */
shared_ptr<NodeInformation> NodeTable::LookUpNodeInformation(const NodeID &id)
{
    shared_ptr<NodeInformation> result = nullptr;
    for(auto i:kBucket[xordistance(id,MyNodeID)])
        if(i->nodeID == id) result = i;
    return result;
}

/**
 * @brief 向表中添加一个新的节点。如果节点已经存在，就把它移到最后面。
 * 
 * @param newNode 待加入的节点
 */
void NodeTable::addNode(shared_ptr<NodeInformation> newNode)
{
    size_t dist = xordistance(newNode->nodeID,MyNodeID);
    auto it = find_if(kBucket[dist].begin(),kBucket[dist].end(),[newNode](const shared_ptr<NodeInformation> item){
        return item->nodeID == newNode->nodeID;
    });
    if(it != kBucket[dist].end())
    {
        //logger.Log(LogLevel::debug,"adding existing nodes"+char2hex(newNode->nodeID.data(),32)+", so move it to the end.");
        auto ni = *it;
        it = kBucket[dist].erase(it);
        kBucket[dist].push_back(ni);
        return;
    }
    else
    {
        kBucket[dist].push_back(newNode);
    }
}

} // namespace Prometheus