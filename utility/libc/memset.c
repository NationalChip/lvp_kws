/*
 * memset.c
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

void *memset(void *dst, int c, size_t n)
{
	char *q = dst;

#if defined(__i386__)
	size_t nl = n >> 2;
	asm volatile ("cld ; rep ; stosl ; movl %3,%0 ; rep ; stosb"
		      : "+c" (nl), "+D" (q)
		      : "a" ((unsigned char)c * 0x01010101U), "r" (n & 3));
#elif defined(__x86_64__)
	size_t nq = n >> 3;
	asm volatile ("cld ; rep ; stosq ; movl %3,%%ecx ; rep ; stosb"
		      :"+c" (nq), "+D" (q)
		      : "a" ((unsigned char)c * 0x0101010101010101U),
			"r" ((uint32_t) n & 7));
#else
	int i;
	unsigned long buffer;
	unsigned long *aligned_addr;
	unsigned int d = c & 0xff;	/* To avoid sign extension, copy C to an
					   unsigned variable.  */

	if (!TOO_SMALL (n) && !UNALIGNED1 (dst))
	{
		/* If we get this far, we know that n is large and m is word-aligned. */
		aligned_addr = (unsigned long*)dst;

		/* Store D into each char sized location in BUFFER so that
		   we can set large blocks quickly.  */
		if (LITTLEBLOCKSIZE == 4)
		{
			buffer = (d << 8) | d;
			buffer |= (buffer << 16);
		}
		else
		{
			buffer = 0;
			for (i = 0; i < LITTLEBLOCKSIZE; i++)
				buffer = (buffer << 8) | d;
		}

		while (n >= LITTLEBLOCKSIZE*4)
		{
			*aligned_addr++ = buffer;
			*aligned_addr++ = buffer;
			*aligned_addr++ = buffer;
			*aligned_addr++ = buffer;
			n -= 4*LITTLEBLOCKSIZE;
		}

		while (n >= LITTLEBLOCKSIZE)
		{
			*aligned_addr++ = buffer;
			n -= LITTLEBLOCKSIZE;
		}
		/* Pick up the remainder with a bytewise loop.  */
		q = (char*)aligned_addr;
	}

	while (n--)
	{
		*q++ = (char)d;
	}
#endif

	return dst;
}
