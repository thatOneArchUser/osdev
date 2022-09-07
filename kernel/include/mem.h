/*
    Copyright (c) 2022 thatOneArchUser
    All rights reserverd
*/

#pragma once

#include <types.h>

#define nullptr ((void *) 0)

void memory_copy(u8* source, u8* dest, u32 nbytes);
void init_dynamic_mem();
void print_dynamic_node_size();
void print_dynamic_mem();
void *mem_alloc(size_t size);
void mem_free(void* p);
void *alloc(TYPE_t t, int n);