/*
 * File: compress.c
 *
 * This is a very quick and compact compression algorithm, called "lzf", and
 * this implementation is "liblzf 2.0", found at <http://liblzf.plan9.de/>.
 * The original files have been compacted into one, and thus only one licence
 * is present in this file.
 *
 * Copyright (c) 2000-2005 Marc Alexander Lehmann <schmorp@schmorp.de>
 * 
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 * 
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 * 
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *   3.  The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License version 2 (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of the above. If you wish to
 * allow the use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * BSD license, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL. If
 * you do not delete the provisions above, a recipient may use your version
 * of this file under either the BSD or the GPL.
 */


/*
 * Changes by -AS-:
 * - removed mention of STRICT_ALIGN, USE_MEMCPY, INIT_HTAB
 * - since Angband has its own fixed-size types, use those
 */

#include "compress.h"

/*** lzfP.h ***/

/*
 * Size of hashtable is (1 << HLOG) * sizeof (char *)
 * decompression is independent of the hash table size
 * the difference between 15 and 14 is very small
 * for small blocks (and 14 is usually a bit faster).
 * For a low-memory/faster configuration, use HLOG == 13;
 * For best compression, use 15 or 16 (or more).
 */
#ifndef HLOG
# define HLOG 15
#endif

/*
 * Sacrifice very little compression quality in favour of compression speed.
 * This gives almost the same compression as the default code, and is
 * (very roughly) 15% faster. This is the preferable mode of operation.
 */

#ifndef VERY_FAST
# define VERY_FAST 1
#endif

/*
 * Sacrifice some more compression quality in favour of compression speed.
 * (roughly 1-2% worse compression for large blocks and
 * 9-10% for small, redundant, blocks and >>20% better speed in both cases)
 * In short: when in need for speed, enable this for binary data,
 * possibly disable this for text data.
 */
#ifndef ULTRA_FAST
# define ULTRA_FAST 0
#endif

/*
 * Avoid assigning values to errno variable? for some embedding purposes
 * (linux kernel for example), this is neccessary. NOTE: this breaks
 * the documentation in lzf.h.
 */
#ifndef AVOID_ERRNO
# define AVOID_ERRNO 0
#endif

/*
 * Wether to pass the LZF_STATE variable as argument, or allocate it
 * on the stack. For small-stack environments, define this to 1.
 * NOTE: this breaks the prototype in lzf.h.
 */
#ifndef LZF_STATE_ARG
# define LZF_STATE_ARG 0
#endif

/*
 * Wether to add extra checks for input validity in lzf_decompress
 * and return EINVAL if the input stream has been corrupted. This
 * only shields against overflowing the input buffer and will not
 * detect most corrupted streams.
 * This check is not normally noticable on modern hardware
 * (<1% slowdown), but might slow down older cpus considerably.
 */
#ifndef CHECK_INPUT
# define CHECK_INPUT 1
#endif


/* Set up types -AS- */
#include "h-basic.h"
#define u8 byte
#define u16 u16b

typedef const u8 *LZF_STATE[1 << (HLOG)];





/*** lzf_c.c ***/

#define HSIZE (1 << (HLOG))

/*
 * don't play with this unless you benchmark!
 * decompression is not dependent on the hash function
 * the hashing function might seem strange, just believe me
 * it works ;)
 */
#ifndef FRST
# define FRST(p) (((p[0]) << 8) | p[1])
# define NEXT(v,p) (((v) << 8) | p[2])
# define IDX(h) ((((h ^ (h << 5)) >> (3*8 - HLOG)) - h*5) & (HSIZE - 1))
#endif
/*
 * IDX works because it is very similar to a multiplicative hash, e.g.
 * ((h * 57321 >> (3*8 - HLOG)) & (HSIZE - 1))
 * the latter is also quite fast on newer CPUs, and sligthly better
 *
 * the next one is also quite good, albeit slow ;)
 * (int)(cos(h & 0xffffff) * 1e6)
 */

#if 0
/* original lzv-like hash function, much worse and thus slower */
# define FRST(p) (p[0] << 5) ^ p[1]
# define NEXT(v,p) ((v) << 5) ^ p[2]
# define IDX(h) ((h) & (HSIZE - 1))
#endif

#define        MAX_LIT        (1 <<  5)
#define        MAX_OFF        (1 << 13)
#define        MAX_REF        ((1 <<  8) + (1 << 3))

/*
 * compressed format
 *
 * 000LLLLL <L+1>    ; literal
 * LLLooooo oooooooo ; backref L
 * 111ooooo LLLLLLLL oooooooo ; backref L+7
 *
 */

unsigned int
lzf_compress (const void *const in_data, unsigned int in_len,
	      void *out_data, unsigned int out_len
#if LZF_STATE_ARG
              , LZF_STATE htab
#endif
              )
{
#if !LZF_STATE_ARG
  LZF_STATE htab;
#endif
  const u8 **hslot;
  const u8 *ip = (const u8 *)in_data;
        u8 *op = (u8 *)out_data;
  const u8 *in_end  = ip + in_len;
        u8 *out_end = op + out_len;
  const u8 *ref;

  unsigned int hval = FRST (ip);
  unsigned long off;
           int lit = 0;


  for (;;)
    {
      if (ip < in_end - 2)
        {
          hval = NEXT (hval, ip);
          hslot = htab + IDX (hval);
          ref = *hslot; *hslot = ip;

          if (1
              && (off = ip - ref - 1) < MAX_OFF
              && ip + 4 < in_end
              && ref > (u8 *)in_data
              && ref[0] == ip[0]
              && ref[1] == ip[1]
              && ref[2] == ip[2]
            )
            {
              /* match found at *ref++ */
              unsigned int len = 2;
              unsigned int maxlen = in_end - ip - len;
              maxlen = maxlen > MAX_REF ? MAX_REF : maxlen;

              if (op + lit + 1 + 3 >= out_end)
                return 0;

              do
                len++;
              while (len < maxlen && ref[len] == ip[len]);

              if (lit)
                {
                  *op++ = lit - 1;
                  lit = -lit;
                  do
                    *op++ = ip[lit];
                  while (++lit);
                }

              len -= 2;
              ip++;

              if (len < 7)
                {
                  *op++ = (off >> 8) + (len << 5);
                }
              else
                {
                  *op++ = (off >> 8) + (  7 << 5);
                  *op++ = len - 7;
                }

              *op++ = off;

#if ULTRA_FAST || VERY_FAST
              ip += len;
#if VERY_FAST && !ULTRA_FAST
              --ip;
#endif
              hval = FRST (ip);

              hval = NEXT (hval, ip);
              htab[IDX (hval)] = ip;
              ip++;

#if VERY_FAST && !ULTRA_FAST
              hval = NEXT (hval, ip);
              htab[IDX (hval)] = ip;
              ip++;
#endif
#else
              do
                {
                  hval = NEXT (hval, ip);
                  htab[IDX (hval)] = ip;
                  ip++;
                }
              while (len--);
#endif
              continue;
            }
        }
      else if (ip == in_end)
        break;

      /* one more literal byte we must copy */
      lit++;
      ip++;

      if (lit == MAX_LIT)
        {
          if (op + 1 + MAX_LIT >= out_end)
            return 0;

          *op++ = MAX_LIT - 1;
          lit = -lit;
          do
            *op++ = ip[lit];
          while (++lit);
        }
    }

  if (lit)
    {
      if (op + lit + 1 >= out_end)
	return 0;

      *op++ = lit - 1;
      lit = -lit;
      do
	*op++ = ip[lit];
      while (++lit);
    }

  return op - (u8 *) out_data;
}



/*** lzf_d.c ***/

#if AVOID_ERRNO
# define SET_ERRNO(n)
#else
# include <errno.h>
# define SET_ERRNO(n) errno = (n)
#endif

unsigned int 
lzf_decompress (const void *const in_data,  unsigned int in_len,
                void             *out_data, unsigned int out_len)
{
  u8 const *ip = (const u8 *)in_data;
  u8       *op = (u8 *)out_data;
  u8 const *const in_end  = ip + in_len;
  u8       *const out_end = op + out_len;

  do
    {
      unsigned int ctrl = *ip++;

      if (ctrl < (1 << 5)) /* literal run */
        {
          ctrl++;

          if (op + ctrl > out_end)
            {
              SET_ERRNO (E2BIG);
              return 0;
            }

#if CHECK_INPUT
          if (ip + ctrl > in_end)
            {
              SET_ERRNO (EINVAL);
              return 0;
            }
#endif

          do
            *op++ = *ip++;
          while (--ctrl);
        }
      else /* back reference */
        {
          unsigned int len = ctrl >> 5;

          u8 *ref = op - ((ctrl & 0x1f) << 8) - 1;

#if CHECK_INPUT
          if (ip >= in_end)
            {
              SET_ERRNO (EINVAL);
              return 0;
            }
#endif
          if (len == 7)
            {
              len += *ip++;
#if CHECK_INPUT
              if (ip >= in_end)
                {
                  SET_ERRNO (EINVAL);
                  return 0;
                }
#endif
            }

          ref -= *ip++;

          if (op + len + 2 > out_end)
            {
              SET_ERRNO (E2BIG);
              return 0;
            }

          if (ref < (u8 *)out_data)
            {
              SET_ERRNO (EINVAL);
              return 0;
            }

          *op++ = *ref++;
          *op++ = *ref++;

          do
            *op++ = *ref++;
          while (--len);
        }
    }
  while (ip < in_end);

  return op - (u8 *)out_data;
}
