/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * All Rights Reserved!
 *
 * malloc_test.c: Qmalloc Test
 *
 */

#include <stdlib.h>
#include <string.h>

#include <printf.h>
#include <fakelibc.h>
#include <q_malloc.h>

static unsigned char test_buffer[100 * 1024];

int malloc_test(void)
{
    if (NULL == test_buffer || 0 == sizeof(test_buffer)) return -1;

    struct qm_block* qb = NULL;
    static int first = 0;
    if ( 0 == first ) {
        qb = qm_malloc_init((void *)test_buffer, sizeof(test_buffer));
        first = 1;
    }

    void *tmp = qm_malloc(qb, 1024*50);
    if (NULL == tmp) {
        printf("Malloc 1024*50 Failed!\n");
    }
    else {
        memset(tmp, 0, 1024*50);
        printf("Malloc 1024*50 Success\n");
    }
    qm_free(qb, tmp);

    return 0;
}

