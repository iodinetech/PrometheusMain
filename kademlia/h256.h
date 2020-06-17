#ifndef _H256_H_
#define _H256_H_

#include <crypto++/sha.h>
#include <cstdint>
#include <algorithm>
#include <random>
#include <cstdio>
#include "tools.h"

namespace Prometheus
{

/** @brief generate random number for SHA256
 * 
 **/
class Randomizer{
public:
    //生成随机的32位正整数
    uint32_t getRandomInt32()
    {
        std::random_device engine;
        return (uint32_t)std::uniform_int_distribution<uint32_t>(0,INT_MAX)(engine);
    }

    //生成随机的SHA256
    std::array<unsigned char,32> getRandomSHA256()
    {
        std::random_device engine;
        std::mt19937 gen(engine());
        GenerateRandom(gen);
        return randomData;
    }
private:
    std::array<unsigned char,32> randomData;
    
    template <class Engine>
	void GenerateRandom(Engine& _eng)
	{
		for(auto& i:randomData)
            i = (uint8_t)std::uniform_int_distribution<uint16_t>(0, 255)(_eng); //distribution has operator()
	}
};

/** @brief produce random SHA256 for NodeID.
 * */
class ShaProducer{
public:
    std::array<unsigned char,32> GetRandomSHA256()
    {
        Randomizer r;
        std::array<unsigned char,32> input = r.getRandomSHA256();
        unsigned char output[32];
        ctx.CalculateDigest(output,(const byte *)input.data(),input.size());
        std::array<unsigned char,32> result;
        for(int i = 0; i < 31; i++)
            result[i] = output[i];
        return result;
    }
private:
    CryptoPP::SHA256 ctx;
};

std::array<unsigned char,32> operator^(const std::array<unsigned char,32>& lhs,const std::array<unsigned char,32>& rhs);
bool operator<(const std::array<unsigned char,32>& lhs,const std::array<unsigned char,32>& rhs);
} //namespace Prometheus

#endif