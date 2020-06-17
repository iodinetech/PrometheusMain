/* This file is part of Prometheus.
*/
#include "common.h"
#include <string>
#ifndef _TOOLS_H_
#define _TOOLS_H_

namespace Prometheus
{
//将字节流转换为人类可读的十六进制序列
std::string char2hex(const unsigned char *input, int length);

//将人类可读的十六进制序列转换为字节流
NodeID hex2char(const std::string& input);

size_t xordistance(const NodeID& a,const NodeID& b);

} // namespace Prometheus

#endif