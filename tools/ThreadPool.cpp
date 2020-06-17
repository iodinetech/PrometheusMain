/** @file ThreadPool.cpp
 *  @author 郑聪
 *  @date 2020/3/30
 * 
 * This file is part of Prometheus.
 */

#include "ThreadPool.h"
#include <string>
#include <cstdio>
#include <iostream>

using namespace std;

namespace Prometheus
{

ThreadPool::ThreadPool(const string& nameArg)
                        : mutex_(),
                        notEmpty(),
                        notFull(),
                        name_(nameArg),
                        maxQueueSize(0),
                        isRunning(false)
{}

ThreadPool::~ThreadPool()
{
    if(isRunning)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    isRunning = true;
    threads.reserve(numThreads);
    for(int i = 0; i< numThreads; i++)
    {
        threads.emplace_back(new thread(std::bind(&ThreadPool::runInThread, this),i));
    }
    if(numThreads == 0 && threadInitCallback)
    {
        threadInitCallback();
    }
}

void ThreadPool::stop()
{
    {
        lock_guard<mutex> lock(mutex_);
        isRunning = false;
        notEmpty.notify_all();
    }
    for(auto& t : threads)
        t->join();
}

size_t ThreadPool::queueSize() const
{
    lock_guard<mutex> lock(mutex_);
    return taskQueue.size();
}

void ThreadPool::run(std::function<void()> task)
{
    if(threads.empty())
    {
        task();
    }
    else
    {
        unique_lock<mutex> lock(mutex_);
        while(isFull())
        {
            notFull.wait(lock);
        }
        taskQueue.push_back(std::move(task));
        notEmpty.notify_one();
    }
}

function<void()> ThreadPool::take()
{
    unique_lock<mutex> lock(mutex_);
    while(taskQueue.empty() && isRunning)
    {
        notEmpty.wait(lock);
    }
    function<void()> task;

    if(!taskQueue.empty())
    {
        task = taskQueue.front();
        taskQueue.pop_front();
        if(maxQueueSize > 0)
        {
            notFull.notify_one();
        }
    }
    return task;
}

bool ThreadPool::isFull() const
{
    return maxQueueSize > 0 && taskQueue.size() >= maxQueueSize;
}

void ThreadPool::runInThread()
{
    try
    {
        if(threadInitCallback)
        {
            threadInitCallback();
        }
        while(isRunning)
        {
            function<void()> task(take());
            if(task)
            {
                task();
            }
        }
    }
    catch(std::exception& e)
    {
        fprintf(stderr,"Exception caught in thread pool %s\n reason:%s\n",name_.c_str(),e.what());
    }
}

} // namespace Prometheus
