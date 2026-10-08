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
    cout << "hset 和 hget" << endl;
    redis.flushall();

    // hset 有多种重载, 返回值是新增字段的个数(更新已有字段返回 0)
    auto ret1 = redis.hset("key", "f1", "v1");
    auto ret2 = redis.hset("key", std::make_pair("f2", "v2"));
    auto ret3 = redis.hset("key", {std::make_pair("f3", "v3"), std::make_pair("f4", "v4")});
    cout << ret1 << " " << ret2 << " " << ret3 << endl;

    // 也可以直接用 map 一次性设置
    unordered_map<string, string> data = {
        {"f5", "v5"},
        {"f6", "v6"}
    };
    redis.hset("key", data.begin(), data.end());

    // 覆盖已有字段, 返回 0
    cout << "覆盖 f1: " << redis.hset("key", "f1", "new_v1") << endl;

    // hget 返回 OptionalString
    auto v1 = redis.hget("key", "f1");
    if (v1) cout << "f1: " << v1.value() << endl;

    auto v2 = redis.hget("key", "no_such_field");
    if (v2) cout << "no_such_field: " << v2.value() << endl;
    else cout << "no_such_field 不存在" << endl;
}

void test2(sw::redis::Redis& redis)
{
    cout << "hexists 和 hdel 和 hlen 和 hstrlen" << endl;
    redis.flushall();
    redis.hset("key", {std::make_pair("f1", "v1"),
                       std::make_pair("f2", "v2"),
                       std::make_pair("f3", "hello")});

    cout << "hexists f1: " << redis.hexists("key", "f1") << endl;
    cout << "hexists f9: " << redis.hexists("key", "f9") << endl;

    cout << "hlen: " << redis.hlen("key") << endl;
    // hstrlen 拿到的是 value 的长度
    cout << "hstrlen f3: " << redis.hstrlen("key", "f3") << endl;

    // hdel 可以一次删除多个字段, 返回真正删掉的个数
    auto ret = redis.hdel("key", {"f1", "f2", "f9"});
    cout << "hdel: " << ret << endl;
    cout << "hlen: " << redis.hlen("key") << endl;
}

void test3(sw::redis::Redis& redis)
{
    cout << "hkeys 和 hvals 和 hgetall" << endl;
    redis.flushall();
    redis.hset("key", {std::make_pair("f1", "v1"),
                       std::make_pair("f2", "v2"),
                       std::make_pair("f3", "v3")});

    vector<string> fields;
    redis.hkeys("key", std::back_inserter(fields));
    cout << "hkeys: ";
    for (const auto& f : fields) cout << f << " ";
    cout << endl;

    vector<string> values;
    redis.hvals("key", std::back_inserter(values));
    cout << "hvals: ";
    for (const auto& v : values) cout << v << " ";
    cout << endl;

    // hgetall 的输出迭代器元素类型是 pair<string, string>
    unordered_map<string, string> all;
    redis.hgetall("key", std::inserter(all, all.begin()));
    cout << "hgetall: " << endl;
    for (const auto& kv : all)
    {
        cout << "  " << kv.first << " -> " << kv.second << endl;
    }
}

void test4(sw::redis::Redis& redis)
{
    cout << "hmset 和 hmget" << endl;
    redis.flushall();

    redis.hmset("key", {std::make_pair("f1", "v1"), std::make_pair("f2", "v2")});

    unordered_map<string, string> data = {
        {"f3", "v3"},
        {"f4", "v4"}
    };
    redis.hmset("key", data.begin(), data.end());

    // hmget 的结果顺序和请求的字段顺序一致, 不存在的字段对应的是空 Optional
    vector<sw::redis::OptionalString> values;
    redis.hmget("key", {"f1", "f3", "no_such_field"}, std::back_inserter(values));
    for (const auto& v : values)
    {
        if (v) cout << v.value() << endl;
        else cout << "(nil)" << endl;
    }
}

void test5(sw::redis::Redis& redis)
{
    cout << "hincrby 和 hincrbyfloat" << endl;
    redis.flushall();
    redis.hset("key", "count", "100");
    redis.hset("key", "score", "3.5");

    // 整数自增, 传负数就是自减
    cout << "hincrby +10: " << redis.hincrby("key", "count", 10) << endl;
    cout << "hincrby -20: " << redis.hincrby("key", "count", -20) << endl;

    // 浮点数自增
    cout << "hincrbyfloat +1.5: " << redis.hincrbyfloat("key", "score", 1.5) << endl;

    // 字段不存在的时候按 0 起算
    cout << "hincrby 新字段: " << redis.hincrby("key", "new_field", 5) << endl;
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
