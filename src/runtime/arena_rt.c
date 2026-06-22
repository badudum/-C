#include "../include/arena_rt.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MC_HEAP_MAGIC   0xDEADBEEFu
#define MC_FLAG_ACTIVE  1u
#define MC_FLAG_ARENA   2u
#define MC_HEAP_HDR     32u
#define MC_ARENA_MAGIC  0x41524E41u /* 'ARNA' */
#define MC_POOL_MAGIC   0x504F4F4Cu /* 'POOL' */
#define MC_MAX_ALLOC    1048576u

typedef struct {
    uint32_t magic;
    uint32_t pad0;
    uint64_t capacity;
    uint64_t bump;
} mc_arena_hdr_t;

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t free_count;
    uint64_t free_head;
} mc_pool_hdr_t;

static void mc_rt_error(const char *msg)
{
    fputs(msg, stderr);
    exit(1);
}

static void *mc_block_from_user(void *user)
{
    if (!user)
        return NULL;
    return (char *)user - MC_HEAP_HDR;
}

static void mc_init_heap_block(void *block, uint32_t user_size, uint32_t flags)
{
    uint32_t *w = (uint32_t *)block;
    w[0] = MC_HEAP_MAGIC;
    *(uint64_t *)((char *)block + 4) = user_size;
    w[3] = flags;
    w[4] = 0;
    memset((char *)block + 20, 0, 12);
}

static void *mc_user_from_block(void *block)
{
    return (char *)block + MC_HEAP_HDR;
}

static void *mc_arena_user_from_hdr(mc_arena_hdr_t *h)
{
    return (char *)h + sizeof(mc_arena_hdr_t);
}

static void *mc_pool_user_from_hdr(mc_pool_hdr_t *h)
{
    return (char *)h + sizeof(mc_pool_hdr_t);
}

static mc_arena_hdr_t *mc_arena_hdr_from_user(void *user)
{
    if (!user)
        return NULL;
    mc_arena_hdr_t *h = (mc_arena_hdr_t *)((char *)user - sizeof(mc_arena_hdr_t));
    if (h->magic != MC_ARENA_MAGIC)
        return NULL;
    return h;
}

static mc_pool_hdr_t *mc_pool_hdr_from_user(void *user)
{
    if (!user)
        return NULL;
    mc_pool_hdr_t *h = (mc_pool_hdr_t *)((char *)user - sizeof(mc_pool_hdr_t));
    if (h->magic != MC_POOL_MAGIC)
        return NULL;
    return h;
}

static uint64_t mc_align_up(uint64_t n, uint64_t a)
{
    return (n + a - 1u) & ~(a - 1u);
}

uint64_t ArenaCreate(int capacity)
{
    if (capacity <= 0 || (unsigned)capacity > MC_MAX_ALLOC)
        mc_rt_error("Runtime Error: Invalid arena capacity\n");
    size_t total = sizeof(mc_arena_hdr_t) + (size_t)capacity;
    mc_arena_hdr_t *h = (mc_arena_hdr_t *)malloc(total);
    if (!h)
        mc_rt_error("Runtime Error: Arena out of memory\n");
    h->magic = MC_ARENA_MAGIC;
    h->pad0 = 0;
    h->capacity = (uint64_t)capacity;
    h->bump = 0;
    return (uint64_t)(uintptr_t)mc_arena_user_from_hdr(h);
}

uint64_t arenaRent(uint64_t arena_adr, int size)
{
    if (size <= 0 || (unsigned)size > MC_MAX_ALLOC)
        mc_rt_error("Runtime Error: Invalid arena allocation size\n");
    void *user = (void *)(uintptr_t)arena_adr;
    mc_arena_hdr_t *h = mc_arena_hdr_from_user(user);
    if (!h)
        mc_rt_error("Runtime Error: Invalid arena handle\n");
    uint64_t need = mc_align_up((uint64_t)size + MC_HEAP_HDR, 16u);
    if (h->bump + need > h->capacity)
        mc_rt_error("Runtime Error: Arena out of memory\n");
    void *block = (char *)user + h->bump;
    h->bump += need;
    mc_init_heap_block(block, (uint32_t)size, MC_FLAG_ACTIVE | MC_FLAG_ARENA);
    return (uint64_t)(uintptr_t)mc_user_from_block(block);
}

int ArenaReset(uint64_t arena_adr)
{
    void *user = (void *)(uintptr_t)arena_adr;
    mc_arena_hdr_t *h = mc_arena_hdr_from_user(user);
    if (!h)
        mc_rt_error("Runtime Error: Invalid arena handle\n");
    h->bump = 0;
    return 0;
}

int ArenaDestroy(uint64_t arena_adr)
{
    void *user = (void *)(uintptr_t)arena_adr;
    mc_arena_hdr_t *h = mc_arena_hdr_from_user(user);
    if (!h)
        mc_rt_error("Runtime Error: Invalid arena handle\n");
    free(h);
    return 0;
}

uint64_t PoolCreate(int block_size, int block_count)
{
    if (block_size <= 0 || block_count <= 0 ||
        (unsigned)block_size > MC_MAX_ALLOC || block_count > 65536)
        mc_rt_error("Runtime Error: Invalid pool parameters\n");
    uint32_t bs = (uint32_t)block_size;
    uint32_t bc = (uint32_t)block_count;
    uint64_t data_bytes = mc_align_up((uint64_t)bs + MC_HEAP_HDR, 16u) * bc;
    size_t total = sizeof(mc_pool_hdr_t) + (size_t)data_bytes;
    mc_pool_hdr_t *h = (mc_pool_hdr_t *)malloc(total);
    if (!h)
        mc_rt_error("Runtime Error: Pool out of memory\n");
    h->magic = MC_POOL_MAGIC;
    h->block_size = bs;
    h->block_count = bc;
    h->free_count = bc;
    h->free_head = 0;
    char *data = (char *)h + sizeof(mc_pool_hdr_t);
    uint64_t stride = mc_align_up((uint64_t)bs + MC_HEAP_HDR, 16u);
    for (uint32_t i = 0; i < bc; i++) {
        void *block = data + (size_t)(stride * i);
        mc_init_heap_block(block, bs, MC_FLAG_ACTIVE | MC_FLAG_ARENA);
        *(uint64_t *)((char *)block + 20) = (i + 1 < bc) ? (uint64_t)(i + 1) : UINT64_MAX;
    }
    return (uint64_t)(uintptr_t)mc_pool_user_from_hdr(h);
}

uint64_t poolRent(uint64_t pool_adr)
{
    void *user = (void *)(uintptr_t)pool_adr;
    mc_pool_hdr_t *h = mc_pool_hdr_from_user(user);
    if (!h)
        mc_rt_error("Runtime Error: Invalid pool handle\n");
    if (h->free_count == 0)
        mc_rt_error("Runtime Error: Pool exhausted\n");
    char *data = (char *)h + sizeof(mc_pool_hdr_t);
    uint64_t stride = mc_align_up((uint64_t)h->block_size + MC_HEAP_HDR, 16u);
    uint64_t idx = h->free_head;
    if (idx >= h->block_count)
        mc_rt_error("Runtime Error: Pool corrupted\n");
    void *block = data + (size_t)(stride * idx);
    uint64_t next = *(uint64_t *)((char *)block + 20);
    h->free_head = (next == UINT64_MAX) ? h->block_count : next;
    h->free_count--;
    ((uint32_t *)block)[3] = MC_FLAG_ACTIVE | MC_FLAG_ARENA;
    return (uint64_t)(uintptr_t)mc_user_from_block(block);
}

int PoolReset(uint64_t pool_adr)
{
    void *user = (void *)(uintptr_t)pool_adr;
    mc_pool_hdr_t *h = mc_pool_hdr_from_user(user);
    if (!h)
        mc_rt_error("Runtime Error: Invalid pool handle\n");
    char *data = (char *)h + sizeof(mc_pool_hdr_t);
    uint64_t stride = mc_align_up((uint64_t)h->block_size + MC_HEAP_HDR, 16u);
    h->free_count = h->block_count;
    h->free_head = 0;
    for (uint32_t i = 0; i < h->block_count; i++) {
        void *block = data + (size_t)(stride * i);
        mc_init_heap_block(block, h->block_size, MC_FLAG_ACTIVE | MC_FLAG_ARENA);
        *(uint64_t *)((char *)block + 20) =
            (i + 1 < h->block_count) ? (uint64_t)(i + 1) : UINT64_MAX;
    }
    return 0;
}

int PoolDestroy(uint64_t pool_adr)
{
    void *user = (void *)(uintptr_t)pool_adr;
    mc_pool_hdr_t *h = mc_pool_hdr_from_user(user);
    if (!h)
        mc_rt_error("Runtime Error: Invalid pool handle\n");
    free(h);
    return 0;
}
