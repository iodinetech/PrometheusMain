/** @file BlockingQueue.h
 *  @author 郑聪
 *  @date 2020/03/26
 *  实现了基本的阻塞队列。
 */

#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <deque>
#include "noncopyable.h"

namespace Prometheus
{
/** @brief 实现了基本的阻塞队列。
 *  有以下的一些操作：
 *  put(T&)和put(T&&):向队列中添加一个元素。
 *  take():取出队列中的一个元素。
 *  size():获得队列的元素个数。
 */

template<class T>
class BlockingQueue : public noncopyable
{
public:
    BlockingQueue()
    : mutex_(),notEmpty_(),queue()
    {}

    ///<向队列中原子地添加一个元素。
    void put(const T& x)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue.push_back(x);
        notEmpty_.notify_one();
    }

    ///<向队列中原子地添加一个元素。
    void put(T&& x)
    {
        std::scoped_lock lock(mutex_);
        queue.push_back(std::move(x));
        notEmpty_.notify_one();
    }

    ///<从队列中原子地取一个元素。如果队列为空则阻塞。
    T take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(queue.empty())
        {
            notEmpty_.wait(lock);
        }
        T front(std::move(queue.front()));
        queue.pop_front();
        return front;
    }

    ///<获得队列的长度。
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::deque<T> queue;
};

} //namespace Prometheus

#endif