#include "kademlia/h256.h"
#include "kademlia/tools.h"
#include "kademlia/common.h"
#include <iostream>

using namespace Prometheus;
using namespace std;

size_t distance(NodeID& a,NodeID& b)
{
    const size_t distanceTable[]={
        8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    NodeID c = a ^ b;
    size_t result = 256;
    int i = 0;
    for(int i = 0;i<32;i++)
    {
        result -= distanceTable[c[i]];
        if(c[i]>0) break;
    }
    return result;
}

int main()
{
    Randomizer r;
    NodeID a = r.getRandomSHA256(); 
    cout<<distance(a,a)<<endl;
    NodeID b = a;
    a[0] = b[0] ^ 1<<7;
    cout<<distance(a,b)<<endl;
    a[0]=b[0];
    a[31]=b[31]^1;
    cout<<distance(a,b)<<endl;
}