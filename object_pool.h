#ifndef __OBJECT_POOL__
#define __OBJECT_POOL__

typedef struct pool_block {
    struct pool_block *next;  // 指向下一个空闲块（仅空闲时有效）
} pool_block_t;

typedef struct {
    pool_block_t *free_list;  // 指向空闲链表的头
    uint8_t      *pool_mem;   // 内存池起始地址
    size_t        block_size; // 每个块的大小
    size_t        block_count;// 块的总数
} mem_pool_t;

#endif
