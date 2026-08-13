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

void test2(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","1");
    redis.set("key2","2");
    redis.set("key3","3");

    auto ret1 = redis.exists("key1");
    auto ret2 = redis.exists("key5");

    std::cout<<ret1<<" "<<ret2<<std::endl;
}

void test3(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","1");
    redis.set("key2","2");
    redis.set("key3","3");

    auto ret1 = redis.exists("key1");
    auto ret2 = redis.del({"key1","key4"});
    auto ret3 = redis.exists("key1");

    std::cout<<ret1<<" "<<ret2<<" "<<ret3<<std::endl;
}

void test4(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","1");
    redis.set("key2","2");
    redis.set("key3","3");

    
    std::vector<string> ret;
    auto it = std::back_inserter(ret);
    redis.keys("*",it);
    for(const auto& i:ret)
    {
        std::cout<<i<<std::endl;
    }
}
void test5(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","1");
    redis.expire("key1",std::chrono::seconds(10));
    auto ret = redis.ttl("key1");
    std::cout<<ret<<std::endl;
    sleep(1);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ret = redis.ttl("key1");
    std::cout<<ret<<std::endl;
}
void test6(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","1");
    redis.lpush("key2","2");
    redis.hset("hash","a","A");
    redis.sadd("set1","data");
    redis.zadd("zset1","data",9.6);
    
    auto func = [&](std::string str){
        std::cout<<redis.type(str)<<std::endl;
    };
    func("key1");
    func("key2");
    func("hash");
    func("set1");
    func("zset1");
}
int main() {
    
    // 创建 Redis 对象的时候, 需要在构造函数中, 指定 redis 服务器的地址和端口. 
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    
    //test1(redis);
    //test2(redis);
    //test3(redis);
    //test4(redis);
    //test5(redis);
    test6(redis);
    return 0;
}

