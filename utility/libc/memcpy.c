/*
 * memcpy.c
 */

#include <string.h>
#include <stdint.h>

#define BIGBLOCKSIZE    (sizeof (long) << 2)
#define LITTLEBLOCKSIZE (sizeof (long))

#define UNALIGNED1(X)   ((long)X & (LITTLEBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < LITTLEBLOCKSIZE)
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)

#define UNALIGNED2(X, Y) \
	(((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

void *memcpy(void *dst, const void *src, size_t n)
{
	const char *p = src;
	char *q = dst;
#if defined(__i386__)
	size_t nl = n >> 2;
	asm volatile ("cld ; rep ; movsl ; movl %3,%0 ; rep ; movsb":"+c" (nl),
		      "+S"(p), "+D"(q)
		      :"r"(n & 3));
#elif defined(__x86_64__)
	size_t nq = n >> 3;
	asm volatile ("cld ; rep ; movsq ; movl %3,%%ecx ; rep ; movsb":"+c"
		      (nq), "+S"(p), "+D"(q)
		      :"r"((uint32_t) (n & 7)));
#else
	long *aligned_dst;
	const long *aligned_src;
	int   len =  n;

	/* If the size is small, or either SRC or DST is unaligned,
	   then punt into the byte copy loop.  This should be rare.  */
	if (!TOO_SMALL(len) && !UNALIGNED2 (p, q))
	{
		aligned_dst = (long*)q;
		aligned_src = (long*)p;

		/* Copy 4X long words at a time if possible.  */
		while (len >= BIGBLOCKSIZE)
		{
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			len -= BIGBLOCKSIZE;
		}

		/* Copy one long word at a time if possible.  */
		while (len >= LITTLEBLOCKSIZE)
		{
			*aligned_dst++ = *aligned_src++;
			len -= LITTLEBLOCKSIZE;
		}

		/* Pick up any residual with a byte copier.  */
		q = (char*)aligned_dst;
		p = (char*)aligned_src;
	}

	while (len--)
		*q++ = *p++;
#endif

	return dst;
}
