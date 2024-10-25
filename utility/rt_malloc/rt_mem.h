#pragma once
void rt_system_heap_init(void *begin_addr, void *end_addr);
void *rt_malloc(unsigned long size);
void *rt_calloc(unsigned long count, unsigned long size);
void *rt_realloc(void *rmem, unsigned long newsize);
void rt_free(void *rmem);
