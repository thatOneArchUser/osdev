/*
    Copyright (c) 2022 thatOneArchUser
    All rights reserverd
*/

#include <mem.h>
#include <string.h>

#include "../drivers/display.h"

#define DYNAMIC_MEM_TOTAL_SIZE 4*1024
#define DYNAMIC_MEM_NODE_SIZE sizeof(dynamic_mem_node_t)

typedef struct dynamic_mem_node {
    uint32_t size;
    bool used;
    struct dynamic_mem_node *next;
    struct dynamic_mem_node *prev;
} dynamic_mem_node_t;

static uint8_t dynamic_mem_area[DYNAMIC_MEM_TOTAL_SIZE];
static dynamic_mem_node_t *dynamic_mem_start;

void memory_copy(u8 *source, u8 *dest, u32 nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void init_dynamic_mem() {
    dynamic_mem_start = (dynamic_mem_node_t *) dynamic_mem_area;
    dynamic_mem_start->size = DYNAMIC_MEM_TOTAL_SIZE - DYNAMIC_MEM_NODE_SIZE;
    dynamic_mem_start->next = nullptr;
    dynamic_mem_start->prev = nullptr;
}

void print_dynamic_node_size() {
    char node_size_string[256];
    int_to_string(DYNAMIC_MEM_NODE_SIZE, node_size_string);
    print_string("DYNAMIC_MEM_NODE_SIZE = ");
    print_string(node_size_string);
    print_nl();
}

void print_dynamic_mem_node(dynamic_mem_node_t *node) {
    char size_string[256];
    int_to_string(node->size, size_string);
    print_string("{size = ");
    print_string(size_string);
    char used_string[256];
    int_to_string(node->used, used_string);
    print_string("; used = ");
    print_string(used_string);
    print_string("}; ");
}

void print_dynamic_mem() {
    dynamic_mem_node_t *current = dynamic_mem_start;
    print_string("[");
    while (current != nullptr) {
        print_dynamic_mem_node(current);
        current = current->next;
    }
    print_string("]\n");
}

void *find_best_mem_block(dynamic_mem_node_t* dynamic_mem, size_t size) {
    dynamic_mem_node_t *best_mem_block = (dynamic_mem_node_t*) nullptr;
    uint32_t best_mem_block_size = DYNAMIC_MEM_TOTAL_SIZE + 1;
    dynamic_mem_node_t *current_mem_block = dynamic_mem;
    while (current_mem_block) {
        if ((!current_mem_block->used) &&
            (current_mem_block->size >= (size + DYNAMIC_MEM_NODE_SIZE)) &&
            (current_mem_block->size <= best_mem_block_size)) {
            best_mem_block = current_mem_block;
            best_mem_block_size = current_mem_block->size;
        }
        current_mem_block = current_mem_block->next;
    }
    return best_mem_block;
}

void *mem_alloc(size_t size) {
    dynamic_mem_node_t *best_mem_block = (dynamic_mem_node_t*) find_best_mem_block(dynamic_mem_start, size);

    if (best_mem_block != nullptr) {
        best_mem_block->size = best_mem_block->size - size - DYNAMIC_MEM_NODE_SIZE;
        dynamic_mem_node_t *mem_node_allocate = (dynamic_mem_node_t*) (((uint8_t*) best_mem_block) + DYNAMIC_MEM_NODE_SIZE + best_mem_block->size);
        mem_node_allocate->size = size;
        mem_node_allocate->used = true;
        mem_node_allocate->next = best_mem_block->next;
        mem_node_allocate->prev = best_mem_block;

        if (best_mem_block->next != nullptr) {
            best_mem_block->next->prev = mem_node_allocate;
        }
        best_mem_block->next = mem_node_allocate;
        return (void *) ((uint8_t *) mem_node_allocate + DYNAMIC_MEM_NODE_SIZE);
    }

    return nullptr;
}

void *merge_next_node_into_current(dynamic_mem_node_t* current_mem_node) {
    dynamic_mem_node_t *next_mem_node = current_mem_node->next;
    if (next_mem_node != nullptr && !next_mem_node->used) {
        current_mem_node->size += current_mem_node->next->size;
        current_mem_node->size += DYNAMIC_MEM_NODE_SIZE;
        current_mem_node->next = current_mem_node->next->next;
        if (current_mem_node->next != nullptr) {
            current_mem_node->next->prev = current_mem_node;
        }
    }
    return current_mem_node;
}

void *merge_current_node_into_previous(dynamic_mem_node_t* current_mem_node) {
    dynamic_mem_node_t *prev_mem_node = current_mem_node->prev;
    if (prev_mem_node != nullptr && !prev_mem_node->used) {
        prev_mem_node->size += current_mem_node->size;
        prev_mem_node->size += DYNAMIC_MEM_NODE_SIZE;

        prev_mem_node->next = current_mem_node->next;
        if (current_mem_node->next != nullptr) {
            current_mem_node->next->prev = prev_mem_node;
        }
    }
    return current_mem_node;
}

void mem_free(void *p) {
    if (p == nullptr) {
        return;
    }
    dynamic_mem_node_t *current_mem_node = (dynamic_mem_node_t *) ((uint8_t *) p - DYNAMIC_MEM_NODE_SIZE);
    if (current_mem_node == nullptr) {
        return;
    }
    current_mem_node->used = false;
    current_mem_node = merge_next_node_into_current(current_mem_node);
    merge_current_node_into_previous(current_mem_node);
}

void *alloc(TYPE_t t, int n) {
    void *ptr;
    switch (t) {
        case INT: {
            ptr = (int *) mem_alloc(n * sizeof(int));
            break;
        }
        case CHAR: {
            ptr = (char *) mem_alloc(n * sizeof(char));
            break;
        }
        case VOID: {
            ptr = (void *) mem_alloc(n * sizeof(void*));
            break;
        }
        case PCHAR: {
            ptr = (char **) mem_alloc(n * sizeof(char*));
            break;
        }
    }
    if (ptr == nullptr) {
        print_string("Memory not allocated\n");
    }
    return ptr;
}