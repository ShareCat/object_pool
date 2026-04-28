#include "object_pool.h"

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
