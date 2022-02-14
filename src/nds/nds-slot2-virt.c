/**
 * \file nds-slot2-virt.c
 */

#ifndef __3DS__

#include <nds.h>
#include "../z-virt.h"
#include "../z-util.h"
#include "dlmalloc.h"
#include "nds-slot2-ram.h"

static uint32_t mem_alt_ptr_mask, mem_alt_ptr_value;
static mspace mem_alt_mspace;

void mem_init_alt(void) {
	if (ram_init(DETECT_RAM)) {
		mem_alt_ptr_mask = 0xFE000000;
		mem_alt_ptr_value = 0x08000000;

		mem_alt_mspace = create_mspace_with_base(ram_unlock(), ram_size(), 0);
	} else {
		mem_alt_ptr_mask = 0xFF000000;
		mem_alt_ptr_value = 0x06000000;

		/* Update if VRAM allocation in nds-draw.c is changed! */
		mem_alt_mspace = create_mspace_with_base(0x06860000, 0x068A4000 - 0x06860000, 0);
	}
}

#define SZ(uptr)	*((size_t *)((char *)(uptr) - sizeof(size_t)))

void *mem_alloc_alt(size_t len)
{
	char *mem;

	/* Allow allocation of "zero bytes" */
	if (len == 0) return (NULL);
	if (len & 1) len++;

	mem = mspace_malloc(mem_alt_mspace, len + sizeof(size_t));
	if (!mem)
		return mem_alloc(len);

	mem += sizeof(size_t);
	SZ(mem) = len;

	return mem;
}

void *mem_zalloc_alt(size_t len)
{
	void *mem = mem_alloc_alt(len);
	if (len) {
		memset(mem, 0, len);
	}
	return mem;
}

void mem_free_alt(void *p)
{
	if (!p) return;

	if ((((u32) p) & mem_alt_ptr_mask) != mem_alt_ptr_value) {
		mem_free(p);
		return;
	}

	mspace_free(mem_alt_mspace, (char *)p - sizeof(size_t));
}

void *mem_realloc_alt(void *p, size_t len)
{
	char *m = p;

	/* Fail gracefully */
	if (len == 0) return (NULL);
	if (!m) return mem_realloc(p, len);
	if ((((u32) p) & mem_alt_ptr_mask) != mem_alt_ptr_value) return mem_realloc(p, len);

	if (len & 1) len++;

	m = mspace_realloc(mem_alt_mspace, m - sizeof(size_t), len + sizeof(size_t));
	if (!m) {
		m = malloc(len + sizeof(size_t));
		if (!m) quit("Out of Memory!");
		memcpy(m + sizeof(size_t), p, len);
		mspace_free(mem_alt_mspace, p);
	}

	m += sizeof(size_t);
	SZ(m) = len;

	return m;
}

bool mem_is_alt_alloc(void *p) {
	return (((u32) p) & mem_alt_ptr_mask) == mem_alt_ptr_value;
}

#endif
