/** @file Connector.h
 *  @author 郑聪
 *  @date 2020/04/04
 * 
 *  This file is part of Prometheus.
 */
#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include "../tools/Logger.h"
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <memory.h>
#include "../tools/BlockingQueue.h"
#include "../net/MessageDispacher.h"
#include <string>
#include <array>
#include <functional>
#include <mutex>
#include <kademlia/common.h>


namespace Prometheus
{
/** @brief 对TCP socket的shared_ptr包装
 */
/*
class TCPConnector : public std::enable_shared_from_this<TCPConnector>
{
public:
    TCPConnector(ba::io_service& service):
        socket_(service),logger()
    {}
    ~TCPConnector()
    {
        socket_.shutdown(bi::tcp::socket::shutdown_both);
        if(socket_.is_open())
            socket_.close();
    }
    bi::tcp::endpoint getEndpoint(){ boost::system::error_code ec; return socket_.remote_endpoint(ec); }
    void fileInfoHandler(boost::system::error_code ec, size_t len)
    {

    }
    bi::tcp::socket& get(){ return socket_; }
    void recv()
    {
        socket_.async_receive(ba::buffer(buf),fileInfoHandler);

    }
    std::function<void()> onReceiving;
private:
    bi::tcp::socket socket_;
    std::vector<char> sendbuf;
    std::mutex x_buf;
    bi::tcp::endpoint lep;
    std::array<char,2048> buf;
    Logger logger_;
};
*/
struct UDPPacket
{
    UDPPacket(std::vector<char>& content_, const bi::udp::endpoint& endpoint_)
    :content(content_),_endpoint(endpoint_)
    {
    }
    std::vector<char> content;
    bi::udp::endpoint _endpoint;
    
};

/** @brief 对UDP的包装，内部有一个接收队列和一个发送队列。
 */ 
class UDPConnector: boost::noncopyable{
public:
    UDPConnector(BlockingQueue<UDPPacket>& bqin, BlockingQueue<UDPPacket>& bqout,ba::io_service& is,Logger logger,bi::udp::endpoint ep)
    :inputQueue(bqin),outputQueue(bqout),lep(ep),logger_(logger),s(is)
    {
    }
    void connect()
    {
        logger_.Log(LogLevel::trace,"UDPConnector connecting");
        s.open(bi::udp::v4());
        s.bind(lep);
    }
    void registerCallback(std::function<void()> callback)
    {
        onReceivingCallback = callback;
    }

    void recv()
    {
        logger_.Log(LogLevel::trace,"UDPConnector Receiving");
        //std::lock_guard<std::mutex> lock(x_buf);
        s.async_receive_from(ba::buffer(buf,512),lep,[this](boost::system::error_code ec, size_t len){            
        if(len)
        {
            std::vector<char> content(512);
            ::memcpy(content.data(),buf.data(),512);
            UDPPacket p(content,lep);
            outputQueue.put(p);
            onReceivingCallback();
        }
        recv();
        });
    }
    void run()
    {
        while(true)
        {
            logger_.Log(LogLevel::trace,"UDPConnector running");
            UDPPacket p = inputQueue.take();
            send(p.content,p._endpoint);
        }
    }
    void send(const std::vector<char>& str,bi::udp::endpoint rep)
    {
        logger_.Log(LogLevel::trace,"UDPConnector Sending to"+std::to_string(rep.port()));
        s.async_send_to(ba::buffer(str),rep,[this](boost::system::error_code ec, size_t len){
    });
    }
    
private:
    std::array<char,512> buf;
    std::vector<char> sendbuf;
    std::mutex x_buf;
    std::function<void()> onReceivingCallback;
    BlockingQueue<UDPPacket>& inputQueue;
    BlockingQueue<UDPPacket>& outputQueue;
    bi::udp::endpoint lep;
    Logger logger_;
    bi::udp::socket s;
};
} // namespace Prometheus

#endif