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

// 打印列表所有元素, 后面的用例都会复用
void printList(sw::redis::Redis& redis, const string& key)
{
    vector<string> ret;
    auto it = std::back_inserter(ret);
    redis.lrange(key, 0, -1, it);
    cout << key << ": ";
    for (const auto& i : ret)
    {
        cout << i << " ";
    }
    cout << endl;
}

void test1(sw::redis::Redis& redis)
{
    cout << "lpush 和 rpush 和 lrange" << endl;
    redis.flushall();

    // lpush 头插, 一次插入多个的时候, 越靠后的元素越靠前
    redis.lpush("key", "1");
    redis.lpush("key", {"2", "3", "4"});
    printList(redis, "key");

    // rpush 尾插
    vector<string> values = {"5", "6", "7"};
    redis.rpush("key", values.begin(), values.end());
    printList(redis, "key");

    // lrange 支持负数下标, -1 表示最后一个元素
    vector<string> ret;
    redis.lrange("key", 1, 3, std::back_inserter(ret));
    cout << "lrange [1, 3]: ";
    for (const auto& i : ret) cout << i << " ";
    cout << endl;
}

void test2(sw::redis::Redis& redis)
{
    cout << "lpop 和 rpop" << endl;
    redis.flushall();
    redis.rpush("key", {"1", "2", "3", "4"});
    printList(redis, "key");

    // 返回值是 OptionalString, 列表不存在或者为空的时候返回的是空的 Optional
    auto v1 = redis.lpop("key");
    if (v1) cout << "lpop: " << v1.value() << endl;

    auto v2 = redis.rpop("key");
    if (v2) cout << "rpop: " << v2.value() << endl;

    printList(redis, "key");

    auto v3 = redis.lpop("no_such_key");
    if (v3) cout << "lpop: " << v3.value() << endl;
    else cout << "no_such_key 不存在" << endl;
}

void test3(sw::redis::Redis& redis)
{
    cout << "llen 和 lindex 和 lrem" << endl;
    redis.flushall();
    redis.rpush("key", {"a", "b", "a", "c", "a"});
    printList(redis, "key");

    cout << "llen: " << redis.llen("key") << endl;

    auto v1 = redis.lindex("key", 1);
    if (v1) cout << "lindex 1: " << v1.value() << endl;

    auto v2 = redis.lindex("key", 100);
    if (v2) cout << "lindex 100: " << v2.value() << endl;
    else cout << "lindex 100: 下标越界" << endl;

    // count > 0 从左往右删, count < 0 从右往左删, count == 0 删除全部
    auto ret = redis.lrem("key", 2, "a");
    cout << "lrem 删除个数: " << ret << endl;
    printList(redis, "key");
}

void test4(sw::redis::Redis& redis)
{
    cout << "linsert 和 ltrim" << endl;
    redis.flushall();
    redis.rpush("key", {"1", "2", "3", "4", "5"});
    printList(redis, "key");

    // 第三个参数是基准元素的值, 不是下标
    auto ret1 = redis.linsert("key", sw::redis::InsertPosition::BEFORE, "3", "before3");
    cout << "linsert 之后长度: " << ret1 << endl;
    printList(redis, "key");

    // 基准元素不存在的时候返回 -1
    auto ret2 = redis.linsert("key", sw::redis::InsertPosition::AFTER, "no_such_value", "x");
    cout << "基准元素不存在: " << ret2 << endl;

    // ltrim 只保留区间内的元素, 区间外的都删掉
    redis.ltrim("key", 1, 3);
    printList(redis, "key");
}

void test5(sw::redis::Redis& redis)
{
    cout << "blpop 阻塞版本" << endl;
    redis.flushall();

    // 另起一个线程, 3 秒之后再往列表中插入数据, 观察 blpop 的阻塞效果
    std::thread t([]() {
        sw::redis::Redis redis2("tcp://127.0.0.1:6379");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        redis2.rpush("key", "666");
        cout << "生产者已经插入数据" << endl;
    });

    cout << "开始 blpop, 等待中..." << endl;
    // 返回值是 OptionalStringPair, first 是键名, second 是取到的元素
    auto ret = redis.blpop({"key", "key2"}, std::chrono::seconds(10));
    if (ret) cout << "blpop: " << ret->first << " -> " << ret->second << endl;
    else cout << "blpop 超时" << endl;

    t.join();
}

void test6(sw::redis::Redis& redis)
{
    cout << "rpoplpush" << endl;
    redis.flushall();
    redis.rpush("key1", {"1", "2", "3"});
    redis.rpush("key2", {"a", "b"});
    printList(redis, "key1");
    printList(redis, "key2");

    // 把 key1 的尾部元素弹出, 头插到 key2 中
    auto ret = redis.rpoplpush("key1", "key2");
    if (ret) cout << "rpoplpush: " << ret.value() << endl;
    printList(redis, "key1");
    printList(redis, "key2");
}

int main()
{
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    // test1(redis);
    // test2(redis);
    // test3(redis);
    // test4(redis);
    // test5(redis);
    test6(redis);
    return 0;
}
