/** @file ThreadPool.h
 *  @author 郑聪
 *  @date 2020/03/29
 * 
 *  This file is part of Prometheus.
 */

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "noncopyable.h"

#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <future>
#include <deque>
#include <iostream>

namespace Prometheus
{

/** @brief 实现了一个基本的线程池。
 */
class ThreadPool : public noncopyable
{
public:

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize) { maxQueueSize = maxSize; }
    void setThreadInitCallback(const std::function<void()> cb) {threadInitCallback = cb; }

    void start(int numThreads);
    void stop();

    const std::string& name() const {return name_; }

    size_t queueSize() const;

/*
    template<class T, class... Args>
    void run(T&& f, Args&&... args)
    {
        std::cout<<"Thread pool tries to run a thread.\n";
        using ReturnType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<T>(f), std::forward<Args>(args)...)
        );
        {
            std::cout<<"Thread Pool is trying to push a task into Queue.\n";
            std::lock_guard<std::mutex> lock{mutex_};
            _run(
                [task]()
                {
                    (*task)();
                }
                );
        }
    }
    */
    void run(std::function<void()> f);
private:
    
    bool isFull() const;
    void runInThread();
    std::function<void()> take();

    //一个拥有所有指向线程的指针的数组。用来直接操作线程。
    std::vector<std::unique_ptr<std::thread>> threads; 

    //操作本线程池的mutex
    mutable std::mutex mutex_;

    //通知所有线程任务队列非空
    mutable std::condition_variable notEmpty;

    //通知所有线程任务队列非满
    mutable std::condition_variable notFull;

    //进程初始化后的回调函数
    std::function<void()> threadInitCallback;

    //任务队列。其中的任务要求没有参数，没有返回值。如果有其他需求将会更改。
    std::deque<std::function<void()>> taskQueue;

    //线程池的名字
    std::string name_;

    //任务队列的最大长度
    size_t maxQueueSize;

    //当前是否有线程在运行。
    bool isRunning;



};

} // namespace Prometheus

#endif