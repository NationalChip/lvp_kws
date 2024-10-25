/* Voice Signal Preprocess
 * Copyright (C) 2001-2019 Nationalchip Co., Ltd
 *
 * q_malloc.h: Malloc implement
 *
 */

#ifndef __Q_MALLOC_H__
#define __Q_MALLOC_H__

struct mem_info {
	unsigned long total_size;
	unsigned long free;
	unsigned long used;
	unsigned long real_used; /*used + overhead*/
	unsigned long max_used;
	unsigned long min_frag;
	unsigned long total_frags; /* total fragment no */
};

/* defs*/
#define ROUNDTO		16UL /* size we round to, must be = 2^n  and also
				sizeof(qm_frag)+sizeof(qm_frag_end)
				must be multiple of ROUNDTO!
				*/
#define MIN_FRAG_SIZE	ROUNDTO

#define QM_MALLOC_OPTIMIZE_FACTOR 14UL /*used below */
#define QM_MALLOC_OPTIMIZE  ((unsigned long)(1UL<<QM_MALLOC_OPTIMIZE_FACTOR))
/* size to optimize for,
   (most allocs <= this size),
   must be 2^k */

#define QM_HASH_SIZE ((unsigned long)(QM_MALLOC_OPTIMIZE/ROUNDTO + \
			(sizeof(long)*8-QM_MALLOC_OPTIMIZE_FACTOR)+1))

/* hash structure:
 * 0 .... QM_MALLOC_OPTIMIE/ROUNDTO  - small buckets, size increases with
 *                            ROUNDTO from bucket to bucket
 * +1 .... end -  size = 2^k, big buckets */

struct qm_frag {
	unsigned long size;
	union{
		struct qm_frag* nxt_free;
		long is_free;
	}u;
};

struct qm_frag_end {
	unsigned long size;
	struct qm_frag* prev_free;
};

struct qm_frag_lnk {
	struct qm_frag head;
	struct qm_frag_end tail;
	unsigned long no;
};

struct qm_block {
	unsigned long size; /* total size */
	unsigned long used; /* alloc'ed size*/
	unsigned long real_used; /* used+malloc overhead*/
	unsigned long max_real_used;

	struct qm_frag* first_frag;
	struct qm_frag_end* last_frag_end;

	struct qm_frag_lnk free_hash[QM_HASH_SIZE];
	/*struct qm_frag_end free_lst_end;*/
};

struct qm_block* qm_malloc_init(char* address, unsigned long size);

void* qm_malloc(struct qm_block*, unsigned long size);
void  qm_free(struct qm_block*, void* p);
void* qm_realloc(struct qm_block*, void* p, unsigned long size);

void  qm_status(struct qm_block*);
void  qm_info(struct qm_block*, struct mem_info*);

#endif  /* __Q_MALLOC_H__ */
