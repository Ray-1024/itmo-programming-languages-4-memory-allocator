#include "tests.h"

#include <unistd.h>

#define SIZE_OF_HEAP (16000)

static void *create_heap(size_t initial_size) {
    return heap_init(initial_size);
}

static void free_heap(void *heap) {
    munmap(heap, size_from_capacity((block_capacity) {.bytes = SIZE_OF_HEAP}).bytes);
}

static void debug(size_t test, char *message, void *heap) {
    printf("[Test %zu: %s]\n", test, message);
    if (heap)debug_heap(stdout, heap);
}


static void memory_allocate_test() {
    debug(1, "started", NULL);
    void *allocated_heap = create_heap(SIZE_OF_HEAP);
    if (!allocated_heap) {
        debug(1, "result: failed", allocated_heap);
        return;
    }
    debug(1, "result: success", allocated_heap);
    free_heap(allocated_heap);
}

static void free_one_block_from_many_allocated_test() {
    debug(2, "started", NULL);
    void *allocated_heap = create_heap(SIZE_OF_HEAP);
    if (!allocated_heap) {
        debug(2, "result: failed", allocated_heap);
        return;
    }
    static size_t len = 5;
    static size_t block_len = 512;
    void *blocks[len];
    for (size_t i = 0; i < len; i++) {
        blocks[i] = _malloc(block_len);
        if (!blocks[i]) {
            debug(2, "result: failed", allocated_heap);
            free_heap(allocated_heap);
            return;
        }
    }
    _free(blocks[3]);
    debug(2, "result: success", allocated_heap);
    for (size_t i = 0; i < len; i++)
        _free(blocks[i]);
    free_heap(allocated_heap);
}

static void free_two_blocks_from_many_allocated_test() {
    debug(3, "started", NULL);
    void *allocated_heap = create_heap(SIZE_OF_HEAP);
    if (!allocated_heap) {
        debug(3, "result: failed", allocated_heap);
        return;
    }
    static size_t len = 5;
    static size_t block_len = 512;
    void *blocks[len];
    for (size_t i = 0; i < len; i++) {
        blocks[i] = _malloc(block_len);
        if (!blocks[i]) {
            debug(3, "result: failed", allocated_heap);
            free_heap(allocated_heap);
            return;
        }
    }
    _free(blocks[1]);
    _free(blocks[3]);
    debug(3, "result: success", allocated_heap);
    for (size_t i = 0; i < len; i++)
        _free(blocks[i]);
    free_heap(allocated_heap);
}

static void region_was_extended_test() {
    void *allocated_heap = create_heap(SIZE_OF_HEAP);
    debug(4, "started", allocated_heap);
    if (!allocated_heap) {
        debug(4, "result: failed", allocated_heap);
        return;
    }
    void *block = _malloc(SIZE_OF_HEAP * 2);
    if (!block) {
        debug(4, "result: failed", allocated_heap);
        free_heap(allocated_heap);
        return;
    }
    struct block_header *header = (struct block_header *) allocated_heap;
    if (header->capacity.bytes < SIZE_OF_HEAP * 2) {
        debug(4, "result: failed", allocated_heap);
        free_heap(allocated_heap);
        return;
    }
    debug(4, "result: success", allocated_heap);
    _free(block);
    free_heap(allocated_heap);
}

static void new_region_is_far_from_here_test() {
    void *allocated_heap = create_heap(SIZE_OF_HEAP);
    debug(5, "started", NULL);
    if (!allocated_heap) {
        debug(5, "result: failed", allocated_heap);
        return;
    }
    size_t size_of_firewall = 1000;
    void *firewall = mmap(allocated_heap + SIZE_OF_HEAP, size_of_firewall, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    size_t size_of_block = SIZE_OF_HEAP * 2;
    void *block = _malloc(size_of_block);
    if (!block) {
        debug(5, "result: failed", allocated_heap);
        free_heap(allocated_heap);
        return;
    }
    struct block_header *heap_header = (struct block_header *) allocated_heap;
    if (!heap_header->is_free || heap_header->next->is_free) {
        debug(5, "result: failed", allocated_heap);
        free_heap(allocated_heap);
        munmap(firewall, size_of_firewall);
        free_heap(allocated_heap);
        munmap(block, size_of_block);
        return;
    }
    debug(5, "result: success", allocated_heap);
    _free(block);
    munmap(firewall, size_of_firewall);
    free_heap(allocated_heap);
    munmap(block, size_of_block);
}

void run_tests() {
    memory_allocate_test();
    free_one_block_from_many_allocated_test();
    free_two_blocks_from_many_allocated_test();
    region_was_extended_test();
    new_region_is_far_from_here_test();
}