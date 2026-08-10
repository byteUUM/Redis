#include <iostream>
#include <hiredis/hiredis.h>

int main() {
    // 连接 Redis
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == nullptr || c->err) {
        std::cerr << "连接失败: " << (c ? c->errstr : "无法分配内存") << std::endl;
        return 1;
    }

    // SET
    redisReply *r = (redisReply *)redisCommand(c, "SET %s %s", "name", "redis-cpp");
    std::cout << "SET: " << r->str << std::endl;
    freeReplyObject(r);

    // GET
    r = (redisReply *)redisCommand(c, "GET name");
    std::cout << "GET: " << r->str << std::endl;
    freeReplyObject(r);

    redisFree(c);
    return 0;
}
