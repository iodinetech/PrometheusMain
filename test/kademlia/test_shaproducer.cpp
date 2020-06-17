/* This file is part of Prometheus.
*/

#include "h256.h"

using namespace std;
using namespace Prometheus;

int main()
{
    ShaProducer sp;
    sp.GetRandomSHA256();
    return 0;
}