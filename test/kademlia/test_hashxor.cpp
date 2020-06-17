/* This file is part of Prometheus.
*/

#include "h256.h"
#include "tools.h"
using namespace std;
using namespace Prometheus;

int main()
{
    Randomizer a;
    auto b = a.getRandom();
    auto c = a.getRandom();
    auto d = b ^ c;
    unsigned char e[65] = {0};
    char2hex(e,(unsigned char*)b.data(),32);
    printf("%s\n",e);
    char2hex(e,(unsigned char*)c.data(),32);
    printf("%s\n",e);
    char2hex(e,(unsigned char*)d.data(),32);
    printf("%s\n",e);
    return 0;
}