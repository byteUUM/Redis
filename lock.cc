#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <cstdio>
#include <unistd.h>
#include <sw/redis++/redis++.h>

using std::cout;
using std::endl;
using std::string;

// 释放锁的 Lua 脚本: 只有当锁的值等于自己的 token 时才删除, 避免误删别人的锁
static const string UNLOCK_SCRIPT = R"(
if redis.call('get', KEYS[1]) == ARGV[1] then
    return redis.call('del', KEYS[1])
else
    return 0
end
)";

// 生成一个唯一的 token, 用来标识 "这把锁是我加的"
string make_token()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;

    char buf[96];
    snprintf(buf, sizeof(buf), "%llx-%llx-%lld-%lld",
             dis(gen), dis(gen),
             (long long)getpid(),
             (long long)std::chrono::steady_clock::now().time_since_epoch().count());
    return buf;
}

class DistributedLock
{
public:
    DistributedLock(sw::redis::Redis& redis) : redis_(redis) {}

    // 加锁: SET key token NX PX ttl_ms
    // 原子操作, 只有 key 不存在时才成功, 同时设置过期时间避免死锁
    bool try_lock(const string& key, const string& token, long long ttl_ms)
    {
        return redis_.set(key, token,
                          std::chrono::milliseconds(ttl_ms),
                          sw::redis::UpdateType::NOT_EXIST);
    }

    // 解锁: 用 Lua 脚本保证 "比较值 + 删除" 是原子的
    // 只有当前持有锁的客户端(值 == token)才能删除
    bool unlock(const string& key, const string& token)
    {
        long long ret = redis_.eval<long long>(UNLOCK_SCRIPT, {key}, {token});
        return ret == 1;
    }

private:
    sw::redis::Redis& redis_;
};

void test1(sw::redis::Redis& redis)
{
    redis.flushall();
    DistributedLock lock(redis);

    string key = "lock:order";
    string token = make_token();

    // 1. 加锁成功
    if (lock.try_lock(key, token, 5000)) {
        cout << "1. 加锁成功, token: " << token << endl;
    }

    // 2. 其他客户端(不同 token)抢不到锁
    string other = make_token();
    if (!lock.try_lock(key, other, 5000)) {
        cout << "2. 其他客户端抢锁失败 (符合预期)" << endl;
    }

    // 3. 用错误 token 解锁失败
    if (!lock.unlock(key, other)) {
        cout << "3. 用错误 token 解锁失败 (符合预期)" << endl;
    }

    // 4. 用正确 token 解锁成功
    if (lock.unlock(key, token)) {
        cout << "4. 用正确 token 解锁成功" << endl;
    }

    // 5. 解锁后可以再次加锁
    string again = make_token();
    if (lock.try_lock(key, again, 5000)) {
        cout << "5. 解锁后再次加锁成功" << endl;
    }
    lock.unlock(key, again);
}

void test2(sw::redis::Redis& redis)
{
    redis.flushall();
    DistributedLock lock(redis);

    string key = "lock:order";
    string token = make_token();

    // 加锁, 1 秒后自动过期
    lock.try_lock(key, token, 1000);
    cout << "加锁成功, 锁 1 秒后自动过期" << endl;

    // 立刻抢锁失败
    string other = make_token();
    cout << "立刻抢锁: " << (lock.try_lock(key, other, 5000) ? "成功" : "失败") << endl;

    // 等 1.5 秒, 锁过期后可以抢到
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    cout << "锁过期后抢锁: " << (lock.try_lock(key, other, 5000) ? "成功" : "失败") << endl;

    lock.unlock(key, other);
}

int main()
{
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    test1(redis);
    test2(redis);
    return 0;
}
