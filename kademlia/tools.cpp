/* This file is part of Prometheus.
*/

#include "tools.h"
#include "h256.h"
#include <iostream>

namespace Prometheus
{

/** @brief 将字节流转换为人类可读的十六进制序列
 * @param input 指向输入的字节流的字节指针 
 * @param length 输入的字节流的长度
 * @retval 一个字符串，为化为十六进制字符串的字符序列
 **/
std::string char2hex(const unsigned char *input, int length)
{
    std::string result;
    result.reserve(2 * length + 1);

    int i = 0;
    unsigned char temp;
    while(i < length)
    {
        temp = input[i] / 16;
        if(temp < 10)
        {
            result.push_back(temp + '0');
        }
        else
        {
            result.push_back(temp + 'a' - 10);
        }
        temp = input[i] % 16;
        if(temp < 10)
        {
            result.push_back(temp + '0');
        }
        else
        {
            result.push_back(temp + 'a' - 10);
        }
        i++;
    }
    return result;
}

//输入的十六进制序列的长度必须为64位
NodeID hex2char(const std::string& input)
{
    unsigned length = input.size();
    NodeID result;
    for(int i=0;i<32;i++)
    {
        if(input[2*i]>='0' && input[2*i]<='9')
            result[i] = input[2*i]-'0';
        else if(input[2*i]>='a' && input[2*i]<='f')
            result[i] = input[2*i]-'a'+10;
        result[i]*=16;
        if(input[2*i+1]>='0' && input[2*i+1]<='9')
            result[i] += input[2*i+1]-'0';
        else if(input[2*i+1]>='a' && input[2*i+1]<='f')
            result[i] += input[2*i+1]-'a'+10;
    }
    return result;
}

/** @brief 计算两个NodeID的异或距离，即单个NodeID的位数减去两个NodeID的异或值的前导零的个数。
 * @param a 一个NodeID
 * @param b 另一个NodeID
 */
size_t xordistance(const NodeID& a,const NodeID& b)
{
    const size_t distanceTable[]={
        8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    NodeID c = a ^ b;
    size_t result = 256;
    int i = 0;
    while(c[i] == 0)
    {
        result -= 8;
        i++;
    }
    if (i != 32) result -= distanceTable[c[i]];
    return result;
}
} //namespace Prometheus