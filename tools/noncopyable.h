/** @file noncopyable.h
 *  @author 郑聪
 *  @date 2020/3/29
 *  
 *  This file is part of Prometheus.
 */
#ifndef _NONCOPYABLE_H_
#define _NONCOPYABLE_H_

namespace Prometheus
{

/** @brief 实现了不可拷贝的类。所有不允许拷贝的类都应该继承这个类。
 */
class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace Prometheus

#endif