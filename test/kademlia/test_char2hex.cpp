/* This file is part of Prometheus.
*/

#include "tools.h"
#include <cstdio>
using namespace std;
using namespace Prometheus;

int main()
{
    unsigned char a[256] = {0};
    for(int i = 0;i < 256; i++)
        a[i] = i;
    printf("%s\n",a);
    unsigned char b[513] = {0};
    char2hex(b,a,255);
    printf("%s\n",b);
    return 0;
}