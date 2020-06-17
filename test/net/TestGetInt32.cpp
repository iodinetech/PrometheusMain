#include <vector>
#include <cstdint>
#include <iostream>
using namespace std;
int32_t getInt32(int32_t *p)
{
    int32_t result = *p;
    be32toh(result);
    return result;
}

void writeInt32(int32_t *p, int32_t num)
{
    *p=htobe32(num);
}

int main()
{
    vector<char> test;
    test.reserve(100);
    const char* begin = test.data();
    int32_t wtf = 1;
    int32_t *smg = &wtf;
    char* jb = (char*)smg;
    for(int i=0;i<4;i++)
        cout<<(int)jb[i]<<" ";
    writeInt32((int32_t*)begin,1);
    for(int i = 0;i < 4; i++)
        cout<<(int)test[i]<<" ";
    int32_t result = getInt32((int32_t*)begin);
    jb = (char*)&result;
    for(int i=0;i<4;i++)
        cout<<(int)jb[i]<<" ";
    cout<<result<<endl;
}