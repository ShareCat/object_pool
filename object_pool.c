#include "object_pool.h"

// 临界区宏定义（以 Cortex-M 为例）
#define ENTER_CRITICAL()    uint32_t primask = __get_PRIMASK(); __disable_irq()
#define EXIT_CRITICAL()     __set_PRIMASK(primask)

void pool_init(mem_pool_t *pool, void *mem, size_t block_size, size_t block_count)
{
    uint8_t *ptr;
    size_t i;

    // 确保块大小至少能容纳一个指针
    if (block_size < sizeof(pool_block_t)) {
        block_size = sizeof(pool_block_t);
    }

    // 记录管理信息
    pool->pool_mem    = (uint8_t *)mem;
    pool->block_size  = block_size;
    pool->block_count = block_count;

    // 初始化空闲链表：把所有块串起来
    pool->free_list = NULL;
    ptr = pool->pool_mem + (block_count - 1) * block_size;  // 从最后一个块开始

    for (i = 0; i < block_count; i++) {
        pool_block_t *block = (pool_block_t *)ptr;
        block->next = pool->free_list;
        pool->free_list = block;
        ptr -= block_size;
    }
}

void* pool_alloc(mem_pool_t *pool)
{
    pool_block_t *block;

    // 检查是否还有空闲块
    if (pool->free_list == NULL) {
        return NULL;  // 池子空了
    }

    // 从链表头弹出一个块
    block = pool->free_list;
    pool->free_list = block->next;

    return (void *)block;
}

void pool_free(mem_pool_t *pool, void *ptr)
{
    pool_block_t *block = (pool_block_t *)ptr;

    // 把这个块压回链表头
    block->next = pool->free_list;
    pool->free_list = block;
}

void* pool_alloc_safe(mem_pool_t *pool)
{
    pool_block_t *block;

    ENTER_CRITICAL();

    if (pool->free_list == NULL) {
        EXIT_CRITICAL();
        return NULL;
    }

    block = pool->free_list;
    pool->free_list = block->next;

    EXIT_CRITICAL();

    return (void *)block;
}

#if 1
void pool_free_safe(mem_pool_t *pool, void *ptr)
{
    pool_block_t *block = (pool_block_t *)ptr;

    ENTER_CRITICAL();

    block->next = pool->free_list;
    pool->free_list = block;

    EXIT_CRITICAL();
}
#else
/*
避免重复释放
netbuf_t *buf = netbuf_alloc();
// ... 用了一会儿 ...
netbuf_free(buf);
// ... 后面又 free 了一次（可能是复制粘贴的 bug）...
netbuf_free(buf);  // 灾难！
*/

#define BLOCK_MAGIC_FREE  0xDEADBEEF
#define BLOCK_MAGIC_USED  0xCAFEBABE

typedef struct pool_block {
    uint32_t magic;              // 魔数：标记当前状态
    struct pool_block *next;     // 空闲时有效
} pool_block_t;

void pool_free_safe(mem_pool_t *pool, void *ptr)
{
    pool_block_t *block = (pool_block_t *)ptr;

    // 检查魔数
    if (block->magic != BLOCK_MAGIC_USED) {
        // 要么是重复释放，要么是野指针
        // 可以选择断言、打印错误、或者直接忽略
        assert(0);
        return;
    }

    ENTER_CRITICAL();
    block->magic = BLOCK_MAGIC_FREE;
    block->next = pool->free_list;
    pool->free_list = block;
    EXIT_CRITICAL();
}
#endif

uint32_t pool_get_free_count(mem_pool_t *pool)
{
    uint32_t count = 0;
    pool_block_t *p = pool->free_list;
    while (p) {
        count++;
        p = p->next;
    }
    return count;
}
