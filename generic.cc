#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sw/redis++/redis++.h>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::unordered_map;

void test1(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","1");
    redis.set("key2","2");
    redis.set("key3","3");

    auto value1 = redis.get("key1");
    auto value2 = redis.get("key2");
    auto value3 = redis.get("key3");
    auto value4 = redis.get("key4");
    if(value1) cout<<value1.value()<<endl;
    if(value2) cout<<value2.value()<<endl;
    if(value3) cout<<value3.value()<<endl;
    if(value4) cout<<value4.value()<<endl;

}

int main() {
    
    // 创建 Redis 对象的时候, 需要在构造函数中, 指定 redis 服务器的地址和端口. 
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    
    test1(redis);
    return 0;
}
