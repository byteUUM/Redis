#include <iostream>
#include <vector>
#include <string>
#include <set>
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

// 打印集合中所有元素, 注意集合内部是无序的
void printSet(sw::redis::Redis& redis, const string& key)
{
    vector<string> ret;
    redis.smembers(key, std::back_inserter(ret));
    cout << key << ": ";
    for (const auto& i : ret) cout << i << " ";
    cout << endl;
}

void test1(sw::redis::Redis& redis)
{
    cout << "sadd 和 smembers 和 scard 和 sismember" << endl;
    redis.flushall();

    // 返回值是真正新增的元素个数, 重复的元素不会被算进去
    auto ret1 = redis.sadd("key", "1");
    auto ret2 = redis.sadd("key", {"2", "3", "4"});
    vector<string> values = {"4", "5"};
    auto ret3 = redis.sadd("key", values.begin(), values.end());
    cout << ret1 << " " << ret2 << " " << ret3 << endl;

    printSet(redis, "key");
    cout << "scard: " << redis.scard("key") << endl;
    cout << "sismember 3: " << redis.sismember("key", "3") << endl;
    cout << "sismember 9: " << redis.sismember("key", "9") << endl;
}

void test2(sw::redis::Redis& redis)
{
    cout << "srem 和 spop 和 srandmember" << endl;
    redis.flushall();
    redis.sadd("key", {"1", "2", "3", "4", "5", "6"});
    printSet(redis, "key");

    // srem 返回真正删掉的个数
    cout << "srem: " << redis.srem("key", {"1", "2", "100"}) << endl;
    printSet(redis, "key");

    // spop 随机弹出元素, 是删除操作
    auto v = redis.spop("key");
    if (v) cout << "spop: " << v.value() << endl;

    vector<string> popped;
    redis.spop("key", 2, std::back_inserter(popped));
    cout << "spop 2 个: ";
    for (const auto& i : popped) cout << i << " ";
    cout << endl;
    printSet(redis, "key");

    // srandmember 只是随机取, 不删除元素
    auto r = redis.srandmember("key");
    if (r) cout << "srandmember: " << r.value() << endl;
    printSet(redis, "key");
}

void test3(sw::redis::Redis& redis)
{
    cout << "sinter 和 sunion 和 sdiff" << endl;
    redis.flushall();
    redis.sadd("key1", {"a", "b", "c", "d"});
    redis.sadd("key2", {"c", "d", "e"});
    printSet(redis, "key1");
    printSet(redis, "key2");

    vector<string> inter;
    redis.sinter({"key1", "key2"}, std::back_inserter(inter));
    cout << "交集: ";
    for (const auto& i : inter) cout << i << " ";
    cout << endl;

    // 也可以用 set 接收, 顺序就是有序的了
    std::set<string> uni;
    redis.sunion({"key1", "key2"}, std::inserter(uni, uni.begin()));
    cout << "并集: ";
    for (const auto& i : uni) cout << i << " ";
    cout << endl;

    // 差集是有方向的, key1 - key2
    vector<string> diff;
    redis.sdiff({"key1", "key2"}, std::back_inserter(diff));
    cout << "差集(key1 - key2): ";
    for (const auto& i : diff) cout << i << " ";
    cout << endl;

    diff.clear();
    redis.sdiff({"key2", "key1"}, std::back_inserter(diff));
    cout << "差集(key2 - key1): ";
    for (const auto& i : diff) cout << i << " ";
    cout << endl;
}

void test4(sw::redis::Redis& redis)
{
    cout << "sinterstore 和 sunionstore 和 sdiffstore" << endl;
    redis.flushall();
    redis.sadd("key1", {"a", "b", "c", "d"});
    redis.sadd("key2", {"c", "d", "e"});

    // store 版本把结果直接存到目标 key 中, 返回结果集合的元素个数
    cout << "sinterstore: " << redis.sinterstore("dest1", {"key1", "key2"}) << endl;
    printSet(redis, "dest1");

    cout << "sunionstore: " << redis.sunionstore("dest2", {"key1", "key2"}) << endl;
    printSet(redis, "dest2");

    cout << "sdiffstore: " << redis.sdiffstore("dest3", {"key1", "key2"}) << endl;
    printSet(redis, "dest3");
}

void test5(sw::redis::Redis& redis)
{
    cout << "smove" << endl;
    redis.flushall();
    redis.sadd("key1", {"a", "b", "c"});
    redis.sadd("key2", {"x"});
    printSet(redis, "key1");
    printSet(redis, "key2");

    // 把元素从一个集合移动到另一个集合, 元素不存在的时候返回 false
    cout << "smove a: " << redis.smove("key1", "key2", "a") << endl;
    cout << "smove no_such: " << redis.smove("key1", "key2", "no_such") << endl;
    printSet(redis, "key1");
    printSet(redis, "key2");
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
