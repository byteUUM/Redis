#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sw/redis++/redis++.h>
#include <chrono>
#include <thread>
#include <unistd.h>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::unordered_map;

void test1(sw::redis::Redis& redis)
{
    redis.set("key1","1",std::chrono::seconds(3));
    cout<<redis.ttl("key1")<<endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cout<<redis.ttl("key1")<<endl;
    cout<<redis.get("key1").value()<<endl;
    
}
int main()
{
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    test1(redis);
    return 0;
}