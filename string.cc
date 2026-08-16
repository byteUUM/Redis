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
void test2(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key2","2",std::chrono::seconds(3),sw::redis::UpdateType::NOT_EXIST);
    auto value = redis.get("key2");
    if(value) cout<<"key2 exist, value: "<<*value<<endl;
    else cout<<"key2 not exist"<<endl;
}
void test3(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.mset({std::make_pair("k","1"), std::make_pair("k2","2"), std::make_pair("k3","3")});
    
    auto v1 = redis.get("k");
    if(v1) cout<<v1.value()<<endl;
    
    auto v2 = redis.get("k2");
    if(v2) cout<<v2.value()<<endl;
    
    auto v3 = redis.get("k3");
    if(v3) cout<<v3.value()<<endl;

    unordered_map<string, string> data = {
        {"key1", "111"},
        {"key2", "222"},
        {"key3", "333"}
    };
    redis.mset(data.begin(), data.end());
}

void test4(sw::redis::Redis& redis)
{
    redis.flushall();
    redis.set("key1","abcdefghijklm");
    auto ret = redis.getrange("key1",5,-1);
    if(!ret.empty()) cout<<"key1 exist, value: "<<ret<<endl;
    
    //不支持负数偏移
    redis.setrange("key1",2,"kkkkkk");
    auto value = redis.get("key1");
    if(value) cout<<"key1 exist, value: "<<value.value()<<endl;
}
void test5(sw::redis::Redis& redis) {
    std::cout << "incr 和 decr" << std::endl;
    redis.flushall();
    redis.set("key", "100");

    long long result = redis.incr("key");
    std::cout << "result: " << result << std::endl;

    auto value = redis.get("key");
    std::cout << "value: " << value.value() << std::endl;

    result = redis.decr("key");
    std::cout << "result: " << result << std::endl;

    value = redis.get("key");
    std::cout << "value: " << value.value() << std::endl;
}

int main()
{
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    // test1(redis);
    // test2(redis);
    // test3(redis);
    // test4(redis);
    test5(redis);
    return 0;
}