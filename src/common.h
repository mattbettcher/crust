#pragma once

#include "stdafx.h"

void fatal(const char *fmt, ...);

void *xcalloc(size_t num_elems, size_t elem_size);
void *xrealloc(void *ptr, size_t num_bytes);
void *xmalloc(size_t num_bytes);
void *memdup(void *src, size_t size);

// allocate and return a formatted string
char *strf(const char *fmt, ...);

char *read_file(const char *path);
bool write_file(const char *path, const char *buf, size_t len);

// Stretchy buffers, invented (?) by Sean Barrett

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size);
char *buf__printf(char *buf, const char *fmt, ...);

// Arena allocator

typedef struct Arena {
    char *ptr;
    char *end;
    char **blocks;
} Arena;

#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE (1024 * 1024)

void arena_grow(Arena *arena, size_t min_size);
void *arena_alloc(Arena *arena, size_t size);
void arena_free(Arena *arena);

// Hash map

uint64_t hash_uint64(uint64_t x);
uint64_t hash_ptr(const void *ptr);
uint64_t hash_mix(uint64_t x, uint64_t y);
uint64_t hash_bytes(const void *ptr, size_t len);

typedef struct Map {
    uint64_t *keys;
    uint64_t *vals;
    size_t len;
    size_t cap;
} Map;

uint64_t map_get_uint64_from_uint64(Map *map, uint64_t key);
void map_put_uint64_from_uint64(Map *map, uint64_t key, uint64_t val);
void map_grow(Map *map, size_t new_cap);
void *map_get(Map *map, const void *key);
void map_put(Map *map, const void *key, void *val);
void *map_get_from_uint64(Map *map, uint64_t key);
void map_put_from_uint64(Map *map, uint64_t key, void *val);
uint64_t map_get_uint64(Map *map, void *key);
void map_put_uint64(Map *map, void *key, uint64_t val);

// String interning

typedef struct Intern {
    size_t len;
    struct Intern *next;
    char str[];
} Intern;

Arena intern_arena;
Map interns;
size_t intern_memory_usage;

const char *str_intern_range(const char *start, const char *end);
const char *str_intern(const char *str);
bool str_islower(const char *str);