/** @file Logger.h
 *  @author 郑聪
 *  @date 2020/03/30
 * 
 *  This file is part of Prometheus.
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

namespace Prometheus
{

enum class LogLevel{
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    error = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
    n_levels
};

/** @brief 包装了spdlog的日志类。
 * 使用异步日志。该异步日志使用内建的线程池。
 * 将日志的等级分为5级，并支持多线程的日志。
 */
class Logger
{
public:
    Logger(const std::string& name)
    {
        logger_ = spdlog::stdout_color_mt<spdlog::async_factory>(name);
    }
    void SetLevel(LogLevel level)
    {
        logger_->set_level(static_cast<spdlog::level::level_enum>(level));
    }
    void Log(LogLevel level, const std::string& content)
    {
        if(level == LogLevel::info)
        {
            logger_->trace(content);
        }
        else if(level == LogLevel::debug)
        {
            logger_->debug(content);
        }
        else if(level == LogLevel::trace)
        {
            logger_->trace(content);
        }
        else if(level == LogLevel::error)
        {
            logger_->error(content);
        }
        else if(level == LogLevel::warn)
        {
            logger_->warn(content);
        }
        else if(level == LogLevel::critical)
        {
            logger_->critical(content);
        }
    }
private:
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace Prometheus

#endif