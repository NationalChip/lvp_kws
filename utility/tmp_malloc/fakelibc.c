/* Voice Signal Preprocess
 * Copyright (C) 2001-2019 Nationalchip Co., Ltd
 *
 * fakelibc.c: fake libc
 *
 */

// ctype.c
#include <string.h>
#include <stdio.h>

#include "fakelibc.h"
#include "q_malloc.h"

#define LEN32_BYTE_ALIGN (1 << 5)  //ck610 must addr 32bytes align, len 32bytes align

static int tmp_mem_initd = 0;
static struct qm_block *tmp_mem_block;
#define tmp_pkg_malloc(s)     qm_malloc(tmp_mem_block, (s))
#define tmp_pkg_realloc(p, s) qm_realloc(tmp_mem_block, (p), (s))
#define tmp_pkg_free(p)       qm_free(tmp_mem_block, (p))

void tmp_heap_init(void *startptr, unsigned int size)
{
	if (tmp_mem_initd)
		return;
	tmp_mem_block = qm_malloc_init(startptr, size);
	if (tmp_mem_block == 0){
		printf("Too much pkg memory demanded: %d\n", size);
	}
	tmp_mem_initd = 1;
}

void *tmp_malloc(unsigned int size)
{
	unsigned char *node;
	unsigned char *tmp, *ret;
	unsigned int ssize = sizeof(void *) + size + LEN32_BYTE_ALIGN;
	unsigned int *p;

	node = tmp_pkg_malloc(ssize);
	if(node == NULL){
		return NULL;
	}
	tmp = node + sizeof(void *);
	ret = (unsigned char *)((unsigned int)(tmp + LEN32_BYTE_ALIGN - 1) & (~(unsigned int)( LEN32_BYTE_ALIGN - 1)));
	p   = (unsigned int *) (ret - 4);
	*p  = (unsigned int)node;

	return ret;
}

void tmp_free(void *ptr)
{
	if(ptr){
		void *tmp = (void *)(*(unsigned int *)((unsigned int)ptr - 4));
		if(tmp)
			tmp_pkg_free(tmp);
	}
}

void *tmp_calloc(unsigned int nmemb, unsigned int size)
{
    unsigned char *ptr = tmp_malloc(nmemb * size);
    if(ptr) {
        memset(ptr, 0, nmemb * size);
    }

    return ptr;
}

void *tmp_realloc(void *ptr, unsigned int size)
{
	void *p;

	if (ptr) {
		p = tmp_malloc(size);
		if (p)
			memcpy(p,ptr,size); //may be read overflow
		tmp_free(ptr);
		return p;
	}
	return tmp_malloc(size);
}

