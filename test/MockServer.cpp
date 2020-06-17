#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <functional>
#include "tools/ThreadPool.h"
#include "tools/BlockingQueue.h"
#include <mutex>
using namespace std;
using namespace boost::asio;
using namespace Prometheus;

class MockProducer: boost::noncopyable{
public:
    MockProducer(BlockingQueue<string>& bqout,BlockingQueue<string>& bqin)
    :outQueue(bqout),inQueue(bqin)
    {}
    void run()
    {
        outQueue.put("test");
        sleep(1);
        run();
    }
    void callBack()
    {
        cout<<"Something came"<<endl;
    }
private:
    BlockingQueue<string>& outQueue;
    BlockingQueue<string>& inQueue;
};

class MockServer: boost::noncopyable{
public:
    MockServer(BlockingQueue<string>& bqin, BlockingQueue<string>& bqout,io_service& is,ip::udp::endpoint ep)
    :inputQueue(bqin),outputQueue(bqout),lep(ep),s(is)
    {
    }
    void connect()
    {

        s.open(ip::udp::v4());
        s.bind(lep);
    }
    void registerCallback(function<void()> callback)
    {
        onReceivingCallback = callback;
    }

    void recv()
    {
        //lock_guard<mutex> lock(x_buf);
        s.async_receive_from(buffer(buf,512),lep,[this](boost::system::error_code ec, size_t len){
        if(len)
        {
            string str(buf.data());
            outputQueue.put(str);
            onReceivingCallback();
        }
        recv();
    });
    }
    void run(ip::udp::endpoint rep)
    {
        string str = inputQueue.take();
        send(str,rep);
        sleep(1);
        run(rep);
    }
    void send(const string& str,ip::udp::endpoint rep)
    {
        lock_guard<mutex> lock(x_buf);
        s.async_send_to(buffer(str),rep,[this](boost::system::error_code ec, size_t len){
    });
    }
    
private:
    std::array<char,512> buf;
    mutex x_buf;
    function<void()> onReceivingCallback;
    BlockingQueue<string>& inputQueue;
    BlockingQueue<string>& outputQueue;
    ip::udp::endpoint lep;
    ip::udp::socket s;
};

int main(int argc, char *argv[])
{
    ip::udp::endpoint ep(ip::udp::v4(),atoi(argv[1]));
    BlockingQueue<string> bqin;
    BlockingQueue<string> bqout;
    string fullep(argv[2]);
    ip::udp::endpoint rep(ip::address_v4::from_string(fullep.substr(0,fullep.find_first_of(':'))),atoi(fullep.substr(fullep.find_first_of(':')+1).c_str()));
    io_service iosvc;
    io_service::work w(iosvc);
    ThreadPool tp("mockserver");
    shared_ptr<MockProducer> mp(new MockProducer(bqout,bqin));
    tp.start(3);
    shared_ptr<MockServer> mc(new MockServer(bqout,bqin,iosvc,ep));
    mc->registerCallback(bind(&MockProducer::callBack,mp.get()));
    mc->connect();
    tp.run(bind(&MockProducer::run,mp.get()));
    tp.run(bind(&MockServer::recv,mc.get()));
    tp.run(bind(&MockServer::run,mc.get(),rep));
    iosvc.run();
    return 0;
}