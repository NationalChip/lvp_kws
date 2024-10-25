/* Voice Signal Preprocess
 * Copyright (C) 2001-2019 Nationalchip Co., Ltd
 *
 * stdio.h: MCU libc standard I/O
 *
 */

#ifndef __FAKELIBC_H__
#define __FAKELIBC_H__

void tmp_heap_init(void *startptr, unsigned int size);
void *tmp_malloc(unsigned int size);
void tmp_free(void *ptr);
void *tmp_calloc(unsigned int nmemb, unsigned int size);
void *tmp_realloc(void *ptr, unsigned int size);

#endif  /* __FAKELIBC_H__ */
