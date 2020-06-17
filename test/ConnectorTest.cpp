#include "net/Connector.h"
#include "tools/BlockingQueue.h"
#include "tools/ThreadPool.h"
#include "tools/Logger.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>

using namespace std;
using namespace Prometheus;
using namespace boost::asio;

class MockHost{
public:
    MockHost(BlockingQueue<UDPPacket>& OutQueue_, BlockingQueue<UDPPacket>& inQueue_,Logger& logger)
    :OutQueue(OutQueue_),inQueue(inQueue_),logger_(logger)
    {}
    void send(ip::udp::endpoint& ep)
    {
        UDPPacket p("hello world!",ep);
        OutQueue.put(p);
        sleep(1);
        send(ep);
    }
    void callback()
    {
        UDPPacket p=inQueue.take();
        logger_.Log(LogLevel::debug,p.content);
    }
private:
    BlockingQueue<UDPPacket>& OutQueue;
    BlockingQueue<UDPPacket>& inQueue;
    Logger logger_;
};

int main(int argc, char* argv[])
{
    assert(argc == 3);
    ip::udp::endpoint remoteEp(ip::address::from_string("127.0.0.1"),atoi(argv[1]));
    ip::udp::endpoint localEp(ip::address_v4(),atoi(argv[2]));
    BlockingQueue<UDPPacket> inQueue; //
    BlockingQueue<UDPPacket> outQueue;
    Prometheus::Logger logger("net");
    shared_ptr<MockHost> mh(new MockHost(outQueue,inQueue,logger));
    logger.SetLevel(LogLevel::debug);
    io_service iosvc;
    io_service::work w(iosvc);
    shared_ptr<UDPConnector> c(new UDPConnector(outQueue,inQueue,iosvc,logger,localEp));
    c->registerCallback(bind(&MockHost::callback,mh.get()));
    c->connect();
    ThreadPool tp("net");
    tp.start(3);
    tp.run(bind(&MockHost::send,mh.get(),remoteEp));
    tp.run(bind(&UDPConnector::recv,c.get()));
    tp.run(bind(&UDPConnector::run,c.get()));
    iosvc.run();
    return 0;
}