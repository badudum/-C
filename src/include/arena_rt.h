#ifndef ARENA_RT_H
#define ARENA_RT_H
#include <stdint.h>

uint64_t ArenaCreate(int capacity);
uint64_t arenaRent(uint64_t arena_adr, int size);
int ArenaReset(uint64_t arena_adr);
int ArenaDestroy(uint64_t arena_adr);

uint64_t PoolCreate(int block_size, int block_count);
uint64_t poolRent(uint64_t pool_adr);
int PoolReset(uint64_t pool_adr);
int PoolDestroy(uint64_t pool_adr);

#endif
