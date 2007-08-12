/* For copyright, please see compress.c */

#ifndef INCLUDED_COMPRESS_H
#define INCLUDED_COMPRESS_H

/*
 * Compress in_len bytes stored at the memory block starting at
 * in_data and write the result to out_data, up to a maximum length
 * of out_len bytes.
 *
 * If the output buffer is not large enough or any error occurs
 * return 0, otherwise return the number of bytes used (which might
 * be considerably larger than in_len, so it makes sense to always
 * use out_len == in_len - 1), to ensure _some_ compression, and store
 * the data uncompressed otherwise.
 *
 * lzf_compress might use different algorithms on different systems and
 * even diferent runs, thus might result in different compressed strings
 * depending on the phase of the moon or similar factors. However, all
 * these strings are architecture-independent and will result in the
 * original data when decompressed using lzf_decompress.
 *
 * The buffers must not be overlapping.
 */
unsigned int 
lzf_compress (const void *const in_data,  unsigned int in_len,
              void             *out_data, unsigned int out_len);

/*
 * Decompress data compressed with some version of the lzf_compress
 * function and stored at location in_data and length in_len. The result
 * will be stored at out_data up to a maximum of out_len characters.
 *
 * If the output buffer is not large enough to hold the decompressed
 * data, a 0 is returned and errno is set to E2BIG. Otherwise the number
 * of decompressed bytes (i.e. the original length of the data) is
 * returned.
 *
 * If an error in the compressed data is detected, a zero is returned and
 * errno is set to EINVAL.
 *
 * This function is very fast, about as fast as a copying loop.
 */
unsigned int 
lzf_decompress (const void *const in_data,  unsigned int in_len,
                void             *out_data, unsigned int out_len);

#endif /* !INCLUDED_COMPRESS_H */
