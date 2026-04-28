#include "object_pool.h"

// 定义 64 字节大小的消息结构体
typedef struct {
    uint8_t type;
    uint8_t data[63];
} message_t;

// 静态分配 100 个消息对象的空间
static uint8_t msg_pool_mem[sizeof(message_t) * 100];
static mem_pool_t msg_pool;

void test_pool(void)
{
    // 初始化
    pool_init(&msg_pool, msg_pool_mem, sizeof(message_t), 100);

    // 申请一个消息对象
    message_t *msg1 = (message_t *)pool_alloc(&msg_pool);
    if (msg1 == NULL) {
        printf("Pool empty!\n");
        return;
    }

    // 使用这个对象
    msg1->type = 0x01;
    memset(msg1->data, 0xAA, 63);

    // 用完了，还回去
    pool_free(&msg_pool, msg1);
}

void main(void)
{
    test_pool();
}
