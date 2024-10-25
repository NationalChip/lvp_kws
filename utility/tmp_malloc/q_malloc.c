/* Voice Signal Preprocess
 * Copyright (C) 2001-2019 Nationalchip Co., Ltd
 *
 * q_malloc.c: Malloc implement
 *
 */

#include <string.h>
#include <stdio.h>
#include "q_malloc.h"

#if 0
#define DBG(...)                printf( __VA_ARGS__ )
#else
#define DBG(...)
#endif

/*useful macros*/
#define FRAG_END(f)  \
	((struct qm_frag_end*)((char*)(f)+sizeof(struct qm_frag)+ \
		(f)->size))

#define FRAG_NEXT(f) \
	((struct qm_frag*)((char*)(f)+sizeof(struct qm_frag)+(f)->size+ \
		sizeof(struct qm_frag_end)))

#define FRAG_PREV(f) \
	( (struct qm_frag*) ( ((char*)(f)-sizeof(struct qm_frag_end))- \
			      ((struct qm_frag_end*)((char*)(f)-sizeof(struct qm_frag_end)))->size- \
			      sizeof(struct qm_frag) ) )

#define PREV_FRAG_END(f) \
	((struct qm_frag_end*)((char*)(f)-sizeof(struct qm_frag_end)))


#define FRAG_OVERHEAD	(sizeof(struct qm_frag)+sizeof(struct qm_frag_end))


#define ROUNDTO_MASK	(~((unsigned long)ROUNDTO-1))
#define ROUNDUP(s)		(((s)+(ROUNDTO-1))&ROUNDTO_MASK)
#define ROUNDDOWN(s)	((s)&ROUNDTO_MASK)

/* finds the hash value for s, s=ROUNDTO multiple*/
#define GET_HASH(s)   ( ((unsigned long)(s)<=QM_MALLOC_OPTIMIZE)?\
		(unsigned long)(s)/ROUNDTO: \
		QM_MALLOC_OPTIMIZE/ROUNDTO+big_hash_idx((s))- \
		QM_MALLOC_OPTIMIZE_FACTOR+1 )

#define UN_HASH(h)	( ((unsigned long)(h)<=(QM_MALLOC_OPTIMIZE/ROUNDTO))?\
		(unsigned long)(h)*ROUNDTO: \
		1UL<<((h)-QM_MALLOC_OPTIMIZE/ROUNDTO+\
			QM_MALLOC_OPTIMIZE_FACTOR-1)\
		)


/* mark/test used/unused frags */
#define FRAG_MARK_USED(f)
#define FRAG_CLEAR_USED(f)
#define FRAG_WAS_USED(f)   (1)

/* other frag related defines:
 * MEM_COALESCE_FRAGS 
 * MEM_FRAG_AVOIDANCE
 */

#define MEM_FRAG_AVOIDANCE

/* computes hash number for big buckets*/
inline static unsigned long big_hash_idx(unsigned long s)
{
	int idx;
	/* s is rounded => s = k*2^n (ROUNDTO=2^n) 
	 * index= i such that 2^i > s >= 2^(i-1)
	 *
	 * => index = number of the first non null bit in s*/
	idx = sizeof(long) * 8 - 1;
	for (; !(s & (1UL << (sizeof(long) * 8 - 1))) ; s <<= 1, idx--);
	return idx;
}

static inline void qm_insert_free(struct qm_block* qm, struct qm_frag* frag)
{
	struct qm_frag* f;
	struct qm_frag* prev;
	int hash;

	hash = GET_HASH(frag->size);
	for(f = qm->free_hash[hash].head.u.nxt_free; f != &(qm->free_hash[hash].head);
			f = f->u.nxt_free){
		if (frag->size <= f->size) break;
	}
	/*insert it here*/
	prev = FRAG_END(f)->prev_free;
	prev->u.nxt_free = frag;
	FRAG_END(frag)->prev_free = prev;
	frag->u.nxt_free = f;
	FRAG_END(f)->prev_free = frag;
	qm->free_hash[hash].no++;
}

/* init malloc and return a qm_block*/
struct qm_block* qm_malloc_init(char* address, unsigned long size)
{
	char* start;
	char* end;
	struct qm_block* qm;
	unsigned long init_overhead;
	int h;

	/* make address and size multiple of 8*/
	start = (char*)ROUNDUP((unsigned long) address);
	DBG("QM_OPTIMIZE=%lu, /ROUNDTO=%lu\n",
			QM_MALLOC_OPTIMIZE, QM_MALLOC_OPTIMIZE/ROUNDTO);
	DBG("QM_HASH_SIZE=%lu, qm_block size=%lu\n",
			QM_HASH_SIZE, (long)sizeof(struct qm_block));
	DBG("params (%p, %lu), start=%p\n", address, size, start);
	if (size < start-address) return 0;
	size -= (start - address);
	if (size < (MIN_FRAG_SIZE + FRAG_OVERHEAD)) return 0;
	size = ROUNDDOWN(size);

	init_overhead = ROUNDUP(sizeof(struct qm_block)) + sizeof(struct qm_frag) + sizeof(struct qm_frag_end);
	DBG("size= %lu, init_overhead=%lu\n", size, init_overhead);

	if (size < init_overhead) {
		/* not enough mem to create our control structures !!!*/
        DBG("not enough mem to create our control structures !!!\n");
		return 0;
	}
	end = start + size;
	qm = (struct qm_block*)start;
	memset(qm, 0, sizeof(struct qm_block));
	qm->size = size;
	qm->real_used = init_overhead;
	qm->max_real_used = qm->real_used;
	size -= init_overhead;

	qm->first_frag = (struct qm_frag*)(start + ROUNDUP(sizeof(struct qm_block)));
	qm->last_frag_end = (struct qm_frag_end*)(end - sizeof(struct qm_frag_end));
	/* init initial fragment*/
	qm->first_frag->size = size;
	qm->last_frag_end->size = size;

	/* init free_hash* */
	for (h = 0; h < QM_HASH_SIZE; h++){
		qm->free_hash[h].head.u.nxt_free=&(qm->free_hash[h].head);
		qm->free_hash[h].tail.prev_free=&(qm->free_hash[h].head);
		qm->free_hash[h].head.size = 0;
		qm->free_hash[h].tail.size = 0;
	}

	qm_insert_free(qm, qm->first_frag);

	return qm;
}

static inline void qm_detach_free(struct qm_block* qm, struct qm_frag* frag)
{
	struct qm_frag *prev;
	struct qm_frag *next;

	prev = FRAG_END(frag)->prev_free;
	next = frag->u.nxt_free;
	prev->u.nxt_free = next;
	FRAG_END(next)->prev_free = prev;
}

static inline struct qm_frag* qm_find_free(struct qm_block* qm, unsigned long size, int* h)
{
	int hash;
	struct qm_frag* f;

	for (hash = GET_HASH(size); hash < QM_HASH_SIZE; hash++){
		for (f = qm->free_hash[hash].head.u.nxt_free; 
				f!=&(qm->free_hash[hash].head); f = f->u.nxt_free){
			if (f->size>=size){ *h=hash; return f; }
		}
		/*try in a bigger bucket*/
	}
	/* not found */
	return 0;
}

/* returns 0 on success, -1 on error;
 * new_size < size & rounded-up already!*/
static inline int split_frag(struct qm_block* qm, struct qm_frag* f, unsigned long new_size)
{
	unsigned long rest;
	struct qm_frag* n;
	struct qm_frag_end* end;

	rest=f->size-new_size;
#ifdef MEM_FRAG_AVOIDANCE
	if ((rest> (FRAG_OVERHEAD+QM_MALLOC_OPTIMIZE))||
			(rest>=(FRAG_OVERHEAD+new_size)))/* the residue fragm. is big enough*/
#else
		if (rest>(FRAG_OVERHEAD+MIN_FRAG_SIZE))
#endif
		{
			f->size = new_size;
			/*split the fragment*/
			end = FRAG_END(f);
			end->size = new_size;
			n = (struct qm_frag*)((char*)end + sizeof(struct qm_frag_end));
			n->size = rest-FRAG_OVERHEAD;
			FRAG_END(n)->size = n->size;
			FRAG_CLEAR_USED(n); /* never used */
			qm->real_used += FRAG_OVERHEAD;
			/* reinsert n in free list*/
			qm_insert_free(qm, n);
			return 0;
		}else{
			/* we cannot split this fragment any more */
			return -1;
		}
}

void* qm_malloc(struct qm_block* qm, unsigned long size)
{
	struct qm_frag* f;
	int hash;

	/*size must be a multiple of 8*/
	size = ROUNDUP(size);
	if (size>(qm->size - qm->real_used)) {
        DBG("no enough mem: (%d, %d) !!!\n", size, qm->size - qm->real_used);
        return 0;
    }


	/*search for a suitable free frag*/
	if ((f = qm_find_free(qm, size, &hash))!=0){
		/* we found it!*/
		/*detach it from the free list*/
		qm_detach_free(qm, f);
		/*mark it as "busy"*/
		f->u.is_free = 0;
		qm->free_hash[hash].no--;
		/* we ignore split return */
		split_frag(qm, f, size);
		qm->real_used += f->size;
		qm->used += f->size;
		if (qm->max_real_used < qm->real_used)
			qm->max_real_used = qm->real_used;
		return (char*)f + sizeof(struct qm_frag);
	}
	return 0;
}

#define QM_JOIN_FREE
void qm_free(struct qm_block* qm, void* p)
{
	struct qm_frag* f;
	struct qm_frag* prev;
	struct qm_frag* next;
	unsigned long size;

	if (p == 0) {
		DBG("free(0) called\n");
		return;
	}
	prev = next = 0;
	f=(struct qm_frag*) ((char*)p - sizeof(struct qm_frag));
	size = f->size;
	qm->used -= size;
	qm->real_used -= size;

#ifdef QM_JOIN_FREE
	/* join packets if possible*/
	next = FRAG_NEXT(f);
	if (((char*)next < (char*)qm->last_frag_end) &&( next->u.is_free)){
		/* join */
		qm_detach_free(qm, next);
		size += next->size + FRAG_OVERHEAD;
		qm->real_used -= FRAG_OVERHEAD;
		qm->free_hash[GET_HASH(next->size)].no--; /* FIXME slow */
	}

	if (f > qm->first_frag){
		prev=FRAG_PREV(f);
		/*	(struct qm_frag*)((char*)f - (struct qm_frag_end*)((char*)f-
			sizeof(struct qm_frag_end))->size);*/
		if (prev->u.is_free){
			/*join*/
			qm_detach_free(qm, prev);
			size += prev->size+FRAG_OVERHEAD;
			qm->real_used -= FRAG_OVERHEAD;
			qm->free_hash[GET_HASH(prev->size)].no--; /* FIXME slow */
			f = prev;
		}
	}
	f->size = size;
	FRAG_END(f)->size = f->size;
#endif /* QM_JOIN_FREE*/
	qm_insert_free(qm, f);
}

void* qm_realloc(struct qm_block* qm, void* p, unsigned long size)
{
	struct qm_frag* f;
	unsigned long diff;
	unsigned long orig_size;
	struct qm_frag* n;
	void* ptr;

	if (size == 0) {
		if (p)
			qm_free(qm, p);
		return 0;
	}
	if (p == 0)
		return qm_malloc(qm, size);
	f = (struct qm_frag*) ((char*)p - sizeof(struct qm_frag));
	/* find first acceptable size */
	size = ROUNDUP(size);
	if (f->size > size) {
		orig_size = f->size;
		/* shrink */
		if(split_frag(qm, f, size)!=0){
			/* update used sizes: freed the spitted frag */
			qm->real_used -= (orig_size - f->size - FRAG_OVERHEAD);
			qm->used -= (orig_size - f->size);
		}

	}
	else if (f->size < size) {
		/* grow */
		orig_size = f->size;
		diff = size-f->size;
		n = FRAG_NEXT(f);
		if (((char*)n < (char*)qm->last_frag_end) && (n->u.is_free) && ((n->size + FRAG_OVERHEAD) >= diff)){
			/* join  */
			qm_detach_free(qm, n);
			qm->free_hash[GET_HASH(n->size)].no--; /*FIXME: slow*/
			f->size += n->size+FRAG_OVERHEAD;
			qm->real_used -= FRAG_OVERHEAD;
			FRAG_END(f)->size = f->size;
			/* end checks should be ok */
			/* split it if necessary */
			if (f->size > size ){
				split_frag(qm, f, size);
			}
			qm->real_used += (f->size-orig_size);
			qm->used += (f->size-orig_size);
		}
		else {
			/* could not join => realloc */
			ptr = qm_malloc(qm, size);
			if (ptr) {
				/* copy, need by libssl */
				memcpy(ptr, p, orig_size);
				qm_free(qm, p);
			}
			p=ptr;
		}
	}
	else {
		/* do nothing */
	}
	return p;
}

void qm_status(struct qm_block* qm)
{
	struct qm_frag* f;
	int i,j;
	int h;
	int unused;

	printf("qm_status (%p):\n", qm);
	if (!qm) return;

	printf(" heap size= %lu\n", qm->size);
	printf(" used= %lu, used+overhead=%lu, free=%lu\n",
			qm->used, qm->real_used, qm->size-qm->real_used);
	printf(" max used (+overhead)= %lu\n", qm->max_real_used);

	printf("dumping all alloc'ed. fragments:\n");
	for (f = qm->first_frag, i = 0; (char*)f<(char*)qm->last_frag_end; f = FRAG_NEXT(f), i++){
		if (! f->u.is_free){
			printf("    %3d. %c  address=%p frag=%p size=%lu used=%d\n",
					i, 
					(f->u.is_free)?'a':'N',
					(char*)f+sizeof(struct qm_frag), f, f->size, FRAG_WAS_USED(f));
		}
	}
	printf("dumping free list stats :\n");
	for(h = 0, i = 0; h < QM_HASH_SIZE; h++){
		unused=0;
		for (f = qm->free_hash[h].head.u.nxt_free, j = 0; 
				f != &(qm->free_hash[h].head); f = f->u.nxt_free, i++, j++){
			if (!FRAG_WAS_USED(f)){
				unused++;
			}
		}

		if (j) printf("hash= %3d. fragments no.: %5d, unused: %5d\n"
				"\t\t bucket size: %9lu - %9ld (first %9lu)\n",
				h, j, unused, UN_HASH(h),
				((h <= QM_MALLOC_OPTIMIZE/ROUNDTO) ? 1 : 2) * UN_HASH(h),
				qm->free_hash[h].head.u.nxt_free->size
			     );
		if (j != qm->free_hash[h].no){
			DBG("different free frag. count: %d!=%lu"
					" for hash %3d\n", j, qm->free_hash[h].no, h);
		}

	}
	printf("-----------------------------\n");
}

void qm_info(struct qm_block* qm, struct mem_info* info)
{
	int r;
	long total_frags;

	total_frags=0;
	memset(info,0, sizeof(*info));
	info->total_size = qm->size;
	info->min_frag = MIN_FRAG_SIZE;
	info->free = qm->size - qm->real_used;
	info->used = qm->used;
	info->real_used = qm->real_used;
	info->max_used = qm->max_real_used;
	for(r=0;r<QM_HASH_SIZE; r++){
		total_frags += qm->free_hash[r].no;
	}
	info->total_frags = total_frags;
}

