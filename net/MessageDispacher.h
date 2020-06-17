/** @file MessageDispacher.h
 *  @author 郑聪
 *  @date 2020/04/12
 *  
 *  This file is part of Prometheus.
 */

#ifndef _MESSAGE_DISPACHER_H_
#define _MESSAGE_DISPACHER_H_

#include <google/protobuf/message.h>
#include "tools/noncopyable.h"
#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <functional>
#include <string>
#include <vector>
#include <type_traits>
#include <iostream>

namespace Prometheus{

/** @brief 从一个地址读入32位数据，并且将字节序从网络字节序转换为本机字节序
 *  @param p 指向32位数据的指针
 */

inline int32_t getInt32(int32_t *p)
{
    return be32toh(*p);
}

/** @brief 向指定的地址写入一个网络字节序的32位数据
 * @param p 写入数据的地址
 * @param num 想要写入的数
 */
inline void writeInt32(int32_t *p, int32_t num)
{
    *p=htobe32(num);
}
//为了实现针对不同类型的消息调用不同的回调，设置的接口
class BaseCallback:noncopyable
{
public:
    virtual void onMessage(const std::shared_ptr<google::protobuf::Message>& message,
    const boost::asio::ip::udp::endpoint& ep) const = 0;
    virtual ~BaseCallback() = default;
};

template<class T>
class ConcreteCallback:public BaseCallback
{
public:
    ConcreteCallback(std::function<void(const std::shared_ptr<T>&, const boost::asio::ip::udp::endpoint&)> _callback)
    :callback(_callback)
    {}
    void onMessage(const std::shared_ptr<google::protobuf::Message>& message,
    const boost::asio::ip::udp::endpoint& ep) const
    {
        std::shared_ptr<T> concreteMessage = std::dynamic_pointer_cast<T>(message);
        callback(concreteMessage,ep);
    }
private:
    std::function<void(const std::shared_ptr<T>& message,const boost::asio::ip::udp::endpoint&)> callback;
};
/** @brief protobuf消息分发器
 * 将从网络上收到的protobuf格式消息根据头部的消息类型构造对象，并发送给对应的回调函数。
 * 原理：protobuf生成的消息类中，有一个全局唯一的descriptor.将其和对应的回调函数绑定，组成一个map，
 * 就可以动态地分发消息了。
 * 
 * 消息格式：0~3字节：消息总长度lenm
 *         4~7字节：消息类型长度lent
 *         8~lent+7字节：消息类型
 *         lent+8~lenm-1字节：消息内容
 *       
 */
class MessageDispacher{
public:
    //构造函数，传入的函数用来处理找不到消息的缺省情况
    MessageDispacher(std::function<void(const std::shared_ptr<google::protobuf::Message>&,const boost::asio::ip::udp::endpoint&) >& _default)
    :defaultCallback(_default)
    {}
    MessageDispacher(std::function<void(const std::shared_ptr<google::protobuf::Message>&, const boost::asio::ip::udp::endpoint&)>&& _default)
    :defaultCallback(_default)
    {}

    /** @brief 当收到消息时调用，将会把字节流对象通过protobuf自带的反射机制变为一个对应类型的message对象，并调用消息类型对应的回调函数。
     * @param m 传入的字节流
     * @param ep 消息发送方的端点
     */

    void onMessage(const std::vector<char>& m,const boost::asio::ip::udp::endpoint& ep)
    {
        std::shared_ptr<google::protobuf::Message> messagePtr;
        const char *begin = m.data();
        int32_t namelen = getInt32((int32_t*)begin);
        int32_t len = getInt32((int32_t*)(begin+4));
        std::string name(begin+8,begin+8+namelen-1); //8指len和namelen所占的字节数
        //从protobuf的类型名构造一个该类型对应的消息
        google::protobuf::Message* message = nullptr;
        const google::protobuf::Descriptor* descriptor = 
              google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(name);
        if(descriptor)
        {
            const google::protobuf::Message* prototype =
                  google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
            if(prototype)
            {
                message = prototype->New();
            }
        }
        messagePtr.reset(message);
        //向构造的消息中填充内容并调用对应的回调函数
        if(message)
        {
            message->ParseFromArray(begin+8+namelen,len-namelen-8);
            auto callbackiter = callbackMap.find(message->GetDescriptor());
            if(callbackiter != callbackMap.end())
                callbackiter->second->onMessage(messagePtr,ep);
            else
                this->defaultCallback(messagePtr,ep);
        }
    }

    /** @brief 将一个message对象序列化后进行必要的类型说明和打包，并生成字节流。
     * @param message 待序列化的message对象
     * @param result 生成的字节流对象
     */
    void makeMessage(const google::protobuf::Message& message, std::vector<char>& result)
    {
        try{
        const std::string& name = message.GetDescriptor()->full_name();
        int32_t lent = static_cast<int32_t>(name.size()+1);
        int32_t lenm = lent+static_cast<int32_t>(message.ByteSize())+2*sizeof(lent);
        result.reserve(lenm);
        char *begin = new char[lenm+1];
        memset(begin,0,sizeof(begin));
        writeInt32((int32_t*)begin,lent);
        writeInt32((int32_t*)(begin+4),lenm);
        memcpy(begin+8,name.c_str(),name.length()+1);
        message.SerializeWithCachedSizesToArray((uint8_t*)begin+8+lent);
        for(int i = 0;i<lenm;i++)
            result.push_back(begin[i]);
        delete[] begin;
        }catch(std::exception &e){
            std::cout<<e.what()<<std::endl;
        }
    }
    /** @brief 注册一个消息类型对应的回调函数。
     * @param _callback 一个类型消息的回调函数。
     * 输入的回调函数要求具有以下的两个参数：
     * 一个指向消息的shared_ptr,和消息发送方的端点。
     */
    template <class T>
    void registerCallback(std::function<void(const std::shared_ptr<T>&,const boost::asio::ip::udp::endpoint&)>& _callback)
    {
        std::shared_ptr<ConcreteCallback<T>> cb(new ConcreteCallback<T>(_callback));
        callbackMap[T::descriptor()] = cb;
    }
private:
    //消息类型的描述符及对应回调函数的映射
    std::map<const google::protobuf::Descriptor*,std::shared_ptr<BaseCallback>> callbackMap;

    //默认的回调函数，处理找不到对应类型消息的情况
    std::function<void(const std::shared_ptr<google::protobuf::Message>&,const boost::asio::ip::udp::endpoint&)> defaultCallback;
};

} //namespace Prometheus

#endif