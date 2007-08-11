/*
 * File: z-map.c
 * Purpose: Implement a serializable map type
 *
 * Copyright (c) 2007 Elly
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */
#include "z-virt.h"
#include "z-file.h"
#include "z-smap.h"


/*** Type names ***/

enum { ST_NONE = 0, ST_BOOL, ST_CHAR, ST_BYTE, ST_S16B,
       ST_U16B, ST_S32B, ST_U32B, ST_STR, ST_BLOB };

char *typenames[] = { "NONE", "BOOL", "CHAR", "BYTE", "S16B",
		      "U16B", "S32B", "U32B", "STR", "BLOB" };


/*** Functions ***/

smap_t *smap_new(void)
{
	return ZNEW(smap_t);
}

void smap_free(smap_t *smap)
{
	sentry_t *se = smap->entries;
	sentry_t *next = (se != NULL ? se->next : NULL);

	while (se)
	{
		next = se->next;

		if (se->type == ST_STR)
			FREE(se->value.strval);
		if (se->type == ST_BLOB)
			FREE(se->value.blobval);

		string_free(se->key);
		FREE(se);
		se = next;
	}

	FREE(smap);
}


/*** Setters ***/

static sentry_t *smap_put(smap_t *smap, const char *key, byte type, u32b dlen)
{
	sentry_t *snew = ZNEW(sentry_t);

	snew->type = type;
	snew->keylen = strlen(key) + 1;
	snew->key = string_make(key);
	snew->datalen = dlen;

	if (!smap->entries)
	{
		smap->entries = snew;
	}
	else
	{
		sentry_t *temp = smap->entries;
		while (temp->next)
			temp = temp->next;
		temp->next = snew;
	}

	return snew;
}

void smap_put_bool(smap_t *smap, const char *key, bool val)
{
	(smap_put(smap, key, ST_BOOL, sizeof(val)))->value.boolval = val;
}

void smap_put_char(smap_t *smap, const char *key, char val)
{
	(smap_put(smap, key, ST_CHAR, sizeof(val)))->value.charval = val;
}

void smap_put_byte(smap_t *smap, const char *key, byte val)
{
	(smap_put(smap, key, ST_BYTE, sizeof(val)))->value.byteval = val;
}

void smap_put_s16b(smap_t *smap, const char *key, s16b val)
{
	(smap_put(smap, key, ST_S16B, sizeof(val)))->value.s16bval = val;
}

void smap_put_u16b(smap_t *smap, const char *key, u16b val)
{
	(smap_put(smap, key, ST_U16B, sizeof(val)))->value.u16bval = val;
}

void smap_put_s32b(smap_t *smap, const char *key, s32b val)
{
	(smap_put(smap, key, ST_S32B, sizeof(val)))->value.s32bval = val;
}

void smap_put_u32b(smap_t *smap, const char *key, u32b val)
{
	(smap_put(smap, key, ST_U32B, sizeof(val)))->value.u32bval = val;
}

void smap_put_str(smap_t *smap, const char *key, const char *val)
{
	(smap_put(smap, key, ST_STR, strlen(val) + 1))->value.strval = string_make(val);
}

void smap_put_blob(smap_t *smap, const char *key, void *data, u32b len)
{
	sentry_t *se = smap_put(smap, key, ST_BLOB, len);
	se->value.blobval = mem_alloc(len);
	memcpy(se->value.blobval, data, len);
}


/*** Getters ***/

static sentry_t *smap_get(smap_t *smap, const char *key)
{
	sentry_t *temp = smap->entries;

	while (temp)
	{
		if (!strncmp(temp->key, key, temp->keylen))
			return temp;

		temp = temp->next;
	}

	return temp;
}

bool smap_get_bool(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.boolval);
}

char smap_get_char(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.charval);
}

byte smap_get_byte(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.byteval);
}

s16b smap_get_s16b(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.s16bval);
}

u16b smap_get_u16b(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.u16bval);
}

s32b smap_get_s32b(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.s32bval);
}

u32b smap_get_u32b(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? 0 : se->value.u32bval);
}

char *smap_get_str(smap_t *smap, const char *key)
{
	sentry_t *se = smap_get(smap, key);
	return (!se ? NULL : string_make(se->value.strval));
}

void *smap_get_blob(smap_t *smap, const char *key, u32b *len)
{
	sentry_t *se = smap_get(smap, key);
	void *nb;

	if (!se)
		return NULL;

	nb = mem_alloc(se->datalen);
	memcpy(nb, se->value.blobval, se->datalen);
	*len = se->datalen;

	return nb;
}


/*** Serialising functions ***/

#define memcpy_u32b(where, variable) \
	do { \
		u32b temp = flip_u32b(variable); \
		memcpy(where, &temp, sizeof(temp)); \
	} while (0);

char *smap_tostring(smap_t *smap, u32b *length)
{
	u32b total_size;
	u32b curr_idx = 0;
	sentry_t *se = smap->entries;
	char *newbuf = NULL;


	/* Make room for the total_size field at the start */
	total_size = sizeof(total_size);

	while (se)
	{
		total_size += sizeof(se->type) + sizeof(se->keylen) + sizeof(se->datalen);
		total_size += se->keylen;
		total_size += se->datalen;
		se = se->next;
	}

	newbuf = mem_alloc(total_size);
	memcpy_u32b(newbuf + curr_idx, total_size);
	curr_idx += sizeof(total_size);

	se = smap->entries;
	while (se)
	{
		memcpy(newbuf + curr_idx, &se->type, sizeof(se->type));
		curr_idx += sizeof(se->type);
		
		memcpy_u32b(newbuf + curr_idx, se->keylen);
		curr_idx += sizeof(se->keylen);

		memcpy_u32b(newbuf + curr_idx, se->datalen);
		curr_idx += sizeof(se->datalen);

		memcpy(newbuf + curr_idx, se->key, se->keylen);
		curr_idx += se->keylen;

		switch (se->type)
		{
			case ST_BOOL:
				memcpy(newbuf + curr_idx, &(se->value.boolval), sizeof(bool));
				break;
			case ST_CHAR:
				memcpy(newbuf + curr_idx, &(se->value.charval), sizeof(char));
				break;
			case ST_BYTE:
				memcpy(newbuf + curr_idx, &(se->value.byteval), sizeof(byte));
				break;
			case ST_S16B:
			{
				u16b temp = flip_u16b((u16b) se->value.s16bval);
				memcpy(newbuf + curr_idx, &temp, sizeof(temp));
				break;
			}
			case ST_U16B:
			{
				u16b temp = flip_u16b(se->value.u16bval);
				memcpy(newbuf + curr_idx, &temp, sizeof(temp));
				break;
			}
			case ST_S32B:
			{
				u32b temp = flip_u32b((u32b) se->value.s32bval);
				memcpy(newbuf + curr_idx, &temp, sizeof(temp));
				break;
			}
			case ST_U32B:
			{
				u32b temp = flip_u32b(se->value.u32bval);
				memcpy(newbuf + curr_idx, &temp, sizeof(temp));
				break;
			}
			case ST_STR:
				memcpy(newbuf + curr_idx, se->value.strval, se->datalen);
				break;
			case ST_BLOB:
				memcpy(newbuf + curr_idx, se->value.blobval, se->datalen);
				break;
			default:
				break;
		}

		curr_idx += se->datalen;
		se = se->next;
	}

	*length = total_size;
	return newbuf;
}


smap_t *smap_fromstring(char *string, u32b length)
{
	smap_t *smap = smap_new();
	u32b total_len = 0;
	u32b idx = 0;
	sentry_t *se = NULL;

	char *tmp_key;
	byte tmp_type;
	u32b tmp_klen;
	u32b tmp_dlen;

	memcpy(&total_len, string + idx, sizeof(total_len));
	total_len = flip_u32b(total_len);
	idx += sizeof(total_len);

	/* We should do some checking of total_len against length here */

	while (idx < total_len)
	{
		memcpy(&tmp_type, string + idx, sizeof(tmp_type));
		idx += sizeof(tmp_type);

		memcpy(&tmp_klen, string + idx, sizeof(tmp_klen));
		tmp_klen = flip_u32b(tmp_klen);
		idx += sizeof(tmp_klen);

		memcpy(&tmp_dlen, string + idx, sizeof(tmp_dlen));
		tmp_dlen = flip_u32b(tmp_dlen);
		idx += sizeof(tmp_dlen);

		tmp_key = string_make(string + idx);
		idx += strlen(tmp_key) + 1;

		se = smap_put(smap, tmp_key, tmp_type, tmp_dlen);
		switch (tmp_type)
		{
			case ST_BOOL:
				memcpy(&(se->value.boolval), string + idx, sizeof(bool));
				break;
			case ST_CHAR:
				memcpy(&(se->value.charval), string + idx, sizeof(char));
				break;
			case ST_BYTE:
				memcpy(&(se->value.byteval), string + idx, sizeof(byte));
				break;
			case ST_S16B:
				memcpy(&(se->value.s16bval), string + idx, sizeof(s16b));
				se->value.s16bval = (s16b) flip_u16b((u16b) se->value.s16bval);
				break;
			case ST_U16B:
				memcpy(&(se->value.u16bval), string + idx, sizeof(u16b));
				se->value.u16bval = flip_u16b(se->value.u16bval);
				break;
			case ST_S32B:
				memcpy(&(se->value.s32bval), string + idx, sizeof(s32b));
				se->value.s32bval = (s32b) flip_u32b((u32b) se->value.s32bval);
				break;
			case ST_U32B:
				memcpy(&(se->value.u32bval), string + idx, sizeof(u32b));
				se->value.u32bval = flip_u32b(se->value.u32bval);
				break;
			case ST_STR:
				se->value.strval = mem_alloc(tmp_dlen);
				memcpy(se->value.strval, string + idx, tmp_dlen);
				break;
			case ST_BLOB:
				se->value.blobval = mem_alloc(tmp_dlen);
				memcpy(se->value.blobval, string + idx, tmp_dlen);
				break;
			default:
				break;
		}

		idx += tmp_dlen;
		FREE(tmp_key);
	}

	return smap;
}

void smap_foreach(smap_t *smap, void (*fn)(sentry_t *))
{
	sentry_t *se = smap->entries;
	while (se)
	{
		fn(se);
		se = se->next;
	}
}

static void smap_printe(sentry_t *se)
{
	u32b i = 0;

	printf("      %s %s: ", typenames[se->type], se->key);
	switch (se->type)
	{
		case ST_BOOL:
			printf("%s\n", (se->value.boolval ? "true" : "false"));
			break;
		case ST_CHAR:
			printf("%02x\n", (byte)(se->value.charval));
			break;
		case ST_BYTE:
			printf("%02x\n", se->value.byteval);
			break;
		case ST_S16B:
			printf("%04x\n", (u16b)(se->value.s16bval));
			break;
		case ST_U16B:
			printf("%04x\n", se->value.u16bval);
			break;
		case ST_S32B:
			printf("%08x\n", (int)(se->value.s32bval));
			break;
		case ST_U32B:
			printf("%08x\n", (unsigned int)(se->value.u32bval));
			break;
		case ST_STR:
			printf("%s\n", se->value.strval);
			break;
		case ST_BLOB:
			for (i = 0; i < se->datalen; i++)
				printf("%02x ", ((unsigned char *)(se->value.blobval))[i]);
			printf("\n");
			break;
		default:
			printf("\n");
			break;
	}
}

void smap_print(smap_t *smap)
{
	smap_foreach(smap, smap_printe);
}
