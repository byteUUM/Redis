#include <iostream>
#include <vector>
#include <string>
#include <utility>
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

// 打印有序集合的全部成员和分数
void printZset(sw::redis::Redis& redis, const string& key)
{
    vector<std::pair<string, double>> ret;
    redis.zrange(key, 0, -1, std::back_inserter(ret));
    cout << key << ": ";
    for (const auto& i : ret) cout << i.first << "(" << i.second << ") ";
    cout << endl;
}

void test1(sw::redis::Redis& redis)
{
    cout << "zadd 和 zrange" << endl;
    redis.flushall();

    // 返回值是新增成员的个数
    auto ret1 = redis.zadd("key", "zhangsan", 90);
    // 注意 initializer_list 版本要求所有分数的类型一致, 都得写成 double
    auto ret2 = redis.zadd("key", {std::make_pair("lisi", 80.5),
                                   std::make_pair("wangwu", 95.0)});
    cout << ret1 << " " << ret2 << endl;

    unordered_map<string, double> data = {
        {"zhaoliu", 70},
        {"sunqi", 85}
    };
    redis.zadd("key", data.begin(), data.end());

    // 更新已有成员的分数, 返回 0, 因为不是新增
    cout << "更新已有成员: " << redis.zadd("key", "zhangsan", 99) << endl;

    // 只接收成员名: 输出迭代器元素类型是 string
    vector<string> members;
    redis.zrange("key", 0, -1, std::back_inserter(members));
    cout << "只要成员名: ";
    for (const auto& m : members) cout << m << " ";
    cout << endl;

    // 想同时拿到分数: 输出迭代器元素类型是 pair<string, double>
    printZset(redis, "key");
}

void test2(sw::redis::Redis& redis)
{
    cout << "zcard 和 zcount 和 zscore 和 zrank" << endl;
    redis.flushall();
    redis.zadd("key", {std::make_pair("a", 10.0),
                       std::make_pair("b", 20.0),
                       std::make_pair("c", 30.0),
                       std::make_pair("d", 40.0)});
    printZset(redis, "key");

    cout << "zcard: " << redis.zcard("key") << endl;

    // 区间默认是闭区间, 这里统计 [20, 40] 的成员个数
    auto cnt = redis.zcount("key", sw::redis::BoundedInterval<double>(20, 40,
                                        sw::redis::BoundType::CLOSED));
    cout << "zcount [20, 40]: " << cnt << endl;

    auto score = redis.zscore("key", "b");
    if (score) cout << "zscore b: " << score.value() << endl;

    auto score2 = redis.zscore("key", "no_such_member");
    if (score2) cout << "zscore: " << score2.value() << endl;
    else cout << "no_such_member 不存在" << endl;

    // 排名从 0 开始, zrank 按分数升序, zrevrank 按分数降序
    auto rank = redis.zrank("key", "c");
    if (rank) cout << "zrank c: " << rank.value() << endl;

    auto revrank = redis.zrevrank("key", "c");
    if (revrank) cout << "zrevrank c: " << revrank.value() << endl;
}

void test3(sw::redis::Redis& redis)
{
    cout << "zrangebyscore 各种区间" << endl;
    redis.flushall();
    redis.zadd("key", {std::make_pair("a", 10.0),
                       std::make_pair("b", 20.0),
                       std::make_pair("c", 30.0),
                       std::make_pair("d", 40.0)});
    printZset(redis, "key");

    auto print = [](const string& tag, const vector<std::pair<string, double>>& v) {
        cout << tag << ": ";
        for (const auto& i : v) cout << i.first << "(" << i.second << ") ";
        cout << endl;
    };

    // 闭区间 [20, 30]
    vector<std::pair<string, double>> ret;
    redis.zrangebyscore("key", sw::redis::BoundedInterval<double>(20, 30,
                            sw::redis::BoundType::CLOSED), std::back_inserter(ret));
    print("[20, 30]", ret);

    // 开区间 (10, 40)
    ret.clear();
    redis.zrangebyscore("key", sw::redis::BoundedInterval<double>(10, 40,
                            sw::redis::BoundType::OPEN), std::back_inserter(ret));
    print("(10, 40)", ret);

    // 只有左边界 [30, +inf)
    ret.clear();
    redis.zrangebyscore("key", sw::redis::LeftBoundedInterval<double>(30,
                            sw::redis::BoundType::RIGHT_OPEN), std::back_inserter(ret));
    print("[30, +inf)", ret);

    // 只有右边界 (-inf, 20]
    ret.clear();
    redis.zrangebyscore("key", sw::redis::RightBoundedInterval<double>(20,
                            sw::redis::BoundType::LEFT_OPEN), std::back_inserter(ret));
    print("(-inf, 20]", ret);

    // 无边界 (-inf, +inf)
    ret.clear();
    redis.zrangebyscore("key", sw::redis::UnboundedInterval<double>{},
                        std::back_inserter(ret));
    print("(-inf, +inf)", ret);
}

void test4(sw::redis::Redis& redis)
{
    cout << "zincrby 和 zrem 和 zremrangeby*" << endl;
    redis.flushall();
    redis.zadd("key", {std::make_pair("a", 10.0),
                       std::make_pair("b", 20.0),
                       std::make_pair("c", 30.0),
                       std::make_pair("d", 40.0),
                       std::make_pair("e", 50.0)});
    printZset(redis, "key");

    // 返回增加之后的分数, 传负数就是减少
    cout << "zincrby a +5: " << redis.zincrby("key", 5, "a") << endl;
    cout << "zincrby b -5: " << redis.zincrby("key", -5, "b") << endl;
    printZset(redis, "key");

    cout << "zrem: " << redis.zrem("key", {"a", "no_such_member"}) << endl;
    printZset(redis, "key");

    // 按排名区间删除
    cout << "zremrangebyrank [0, 0]: " << redis.zremrangebyrank("key", 0, 0) << endl;
    printZset(redis, "key");

    // 按分数区间删除
    auto ret = redis.zremrangebyscore("key",
                    sw::redis::BoundedInterval<double>(40, 50, sw::redis::BoundType::CLOSED));
    cout << "zremrangebyscore [40, 50]: " << ret << endl;
    printZset(redis, "key");
}

void test5(sw::redis::Redis& redis)
{
    cout << "zrevrange 和 zpopmax 和 zpopmin" << endl;
    redis.flushall();
    redis.zadd("key", {std::make_pair("a", 10.0),
                       std::make_pair("b", 20.0),
                       std::make_pair("c", 30.0)});
    printZset(redis, "key");

    // 按分数从大到小取
    vector<std::pair<string, double>> ret;
    redis.zrevrange("key", 0, -1, std::back_inserter(ret));
    cout << "zrevrange: ";
    for (const auto& i : ret) cout << i.first << "(" << i.second << ") ";
    cout << endl;

    // 弹出分数最大/最小的成员, 是删除操作
    auto max = redis.zpopmax("key");
    if (max) cout << "zpopmax: " << max->first << "(" << max->second << ")" << endl;

    auto min = redis.zpopmin("key");
    if (min) cout << "zpopmin: " << min->first << "(" << min->second << ")" << endl;

    printZset(redis, "key");
}

void test6(sw::redis::Redis& redis)
{
    cout << "zinterstore 和 zunionstore" << endl;
    redis.flushall();
    redis.zadd("key1", {std::make_pair("a", 1.0), std::make_pair("b", 2.0)});
    redis.zadd("key2", {std::make_pair("b", 3.0), std::make_pair("c", 4.0)});
    printZset(redis, "key1");
    printZset(redis, "key2");

    // 默认权重为 1, 相同成员的分数相加
    cout << "zinterstore: " << redis.zinterstore("dest1", {"key1", "key2"}) << endl;
    printZset(redis, "dest1");

    cout << "zunionstore: " << redis.zunionstore("dest2", {"key1", "key2"}) << endl;
    printZset(redis, "dest2");

    // 带权重, 并且指定聚合方式为取最大值
    vector<std::pair<string, double>> keys_with_weights = {{"key1", 10}, {"key2", 1}};
    auto ret = redis.zunionstore("dest3", keys_with_weights.begin(), keys_with_weights.end(),
                                 sw::redis::Aggregation::MAX);
    cout << "zunionstore 带权重: " << ret << endl;
    printZset(redis, "dest3");
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
