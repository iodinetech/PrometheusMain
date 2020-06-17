/**
 * This file is part of Prometheus.
 * @author 郑聪
 * @date 2020/05/03
**/

#include "kademlia/kademliaHost.h"
#include "kademlia/common.h"
#include "kademlia/tools.h"
#include "kademlia/nodeTable.h"
#include "kademlia/h256.h"
#include "tools/Logger.h"
#include <boost/asio.hpp>
#include <memory>
#include <iostream>

using namespace std;
using namespace Prometheus;

int main()
{
	Randomizer r;
	NodeID myid = r.getRandomSHA256();
	cout<<xordistance(myid,myid)<<endl;
	cout<<char2hex(myid.data(),32)<<endl;
	Logger l("logger");
	vector<NodeID> IDset;
	NodeTable table(myid,l);
	for(int i = 0;i<32;i++)
	{
		for(int j = 7;j>=0;j--)
		{
			NodeID insert = myid;
			insert[i] ^= 1<<j;
			IDset.push_back(insert);
			NodeID temp = insert ^ myid;
			boost::asio::ip::udp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"),1234);
			shared_ptr<NodeInformation> pn(new NodeInformation(insert,ep.address(),ep.port()));
			table.addNode(pn);
		}
	}
	auto bucket = table.getBucket();
	for(auto i:bucket)
	{
		for(auto j:i)
		{
			cout<<char2hex(j->nodeID.data(),32)<<endl;
		}
		cout<<endl;
	}
	cout<<endl<<"input="<<char2hex(IDset[5].data(),32)<<endl;
	auto nearestNodes = table.LookUpKNearestNodes(IDset[5]);
	for(auto i:nearestNodes)
	{
		cout<<char2hex(i->nodeID.data(),32)<<endl;
	}
	nearestNodes = table.LookUpKNearestNodes(myid);
	for(auto i:nearestNodes)
	{
		cout<<char2hex(i->nodeID.data(),32)<<endl;
	}
	nearestNodes = table.LookUpKNearestNodes(IDset[IDset.size()-1]);
	for(auto i:nearestNodes)
	{
		cout<<char2hex(i->nodeID.data(),32)<<endl;
	}
	nearestNodes = table.LookUpKNearestNodes(IDset[IDset.size()-2]);
	for(auto i:nearestNodes)
	{
		cout<<char2hex(i->nodeID.data(),32)<<endl;
	}
	nearestNodes = table.LookUpKNearestNodes(IDset[0]);
	for(auto i:nearestNodes)
	{
		cout<<char2hex(i->nodeID.data(),32)<<endl;
	}
	return 0;
}
