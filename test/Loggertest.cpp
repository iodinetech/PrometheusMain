#include "tools/Logger.h"
#include <string>
#include "tools/ThreadPool.h"
#include <functional>
#include <iostream>

using namespace Prometheus;
using namespace std;

class LoggerTest
{
public:
    LoggerTest(const string& name):
        _name(name),logger(name)
        {}
    void run()
    {
        cout<<"LoggerTest is being run\n";
        log();
    }
    void log()
    {
        logger.SetLevel(LogLevel::trace);
        while(true){
            logger.Log(LogLevel::trace,"this is a debug message from " + _name);
            logger.Log(LogLevel::debug,"this is a debug message from " + _name);
            logger.Log(LogLevel::warn,"this is a warn message from " + _name);
            logger.Log(LogLevel::error,"this is a error message from " + _name);
            logger.Log(LogLevel::critical,"this is a critical message from " + _name);
            logger.Log(LogLevel::info,"this is a info message from " + _name);
            sleep(1);
        }
    }
private:
    string _name;
    Logger logger;
};

int main()
{
    ThreadPool tp("this is a thread poooooool");
    LoggerTest lg1("thread1");
    LoggerTest lg2("thread2");
    LoggerTest lg3("thread3");
    LoggerTest lg4("thread4");
    LoggerTest lg5("thread5");
    LoggerTest lg6("thread6");
    tp.start(6);
    tp.run(std::bind(&LoggerTest::run,&lg1));
    tp.run(std::bind(&LoggerTest::run,&lg2));
    tp.run(std::bind(&LoggerTest::run,&lg3));
    tp.run(std::bind(&LoggerTest::run,&lg4));
    tp.run(std::bind(&LoggerTest::run,&lg5));
    tp.run(std::bind(&LoggerTest::run,&lg6));
}