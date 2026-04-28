#ifndef __OBJECT_POOL__
#define __OBJECT_POOL__

#if 1
#if 1
typedef struct pool_block {
    struct pool_block *next;  // 指向下一个空闲块（仅空闲时有效）
} pool_block_t;
#else
// 有些 CPU 对数据访问有对齐要求。比如 ARM Cortex-M 访问 uint32_t 必须 4 字节对齐，访问 double 必须 8 字节对齐。如果你的块大小是奇数（比如 65 字节），而你在块里存了一个 double，就可能触发 HardFault
// 内存 4 字节对齐
typedef struct {
    uint32_t magic;
    struct pool_block *next;  // 指向下一个空闲块（仅空闲时有效）
} __attribute__((aligned(4))) pool_block_t;
#endif
#else
typedef union {
    struct {
        union pool_block *next;  // 空闲时：指向下一个空闲块
    } free;
    struct {
        uint8_t data[1];         // 使用时：用户数据（柔性数组）
    } used;
} pool_block_t;
#endif

typedef struct {
    pool_block_t *free_list;  // 指向空闲链表的头
    uint8_t      *pool_mem;   // 内存池起始地址
    size_t        block_size; // 每个块的大小
    size_t        block_count;// 块的总数
} mem_pool_t;

extern void pool_init(mem_pool_t *pool, void *mem, size_t block_size, size_t block_count);
extern void* pool_alloc(mem_pool_t *pool);
extern void pool_free(mem_pool_t *pool, void *ptr);
extern void* pool_alloc_safe(mem_pool_t *pool);
extern void pool_free_safe(mem_pool_t *pool, void *ptr);
extern uint32_t pool_get_free_count(mem_pool_t *pool);

#endif
