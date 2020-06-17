#include "h256.h"
#include <array>

namespace Prometheus
{
    
std::array<unsigned char,32> operator^(const std::array<unsigned char,32>& lhs, const std::array<unsigned char,32>& rhs)
{
    std::array<unsigned char,32> result;
    for(int i = 0; i < 32; i++)
        result[i] = lhs[i] ^ rhs[i];
    return result;
}

bool operator<(const std::array<unsigned char,32>& lhs,const std::array<unsigned char,32>& rhs)
{
    bool flag = false;
    for(int i=0;i<32;i++)
    {
        if(lhs[i]<rhs[i])
        {
            flag = true;
            break;
        }
    }
    return flag;
}
} //namespace Prometheus