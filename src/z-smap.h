#ifndef INCLUDED_SERIALIZE_H
#define INCLUDED_SERIALIZE_H

#include "h-basic.h"

typedef struct _sentry_t
{
	byte type;
	byte keylen;
	u32b datalen;
	char *key;
	union
	{
		bool boolval;
		char charval;
		byte byteval;
		s16b s16bval;
		u16b u16bval;
		s32b s32bval;
		u32b u32bval;
		char *strval;
		void *blobval;
	} value;
	
	struct _sentry_t *next;
} sentry_t;

typedef struct _smap_t
{
	sentry_t *entries;
} smap_t;

smap_t *smap_new(void);
void smap_free(smap_t *smap);

void smap_put_bool(smap_t *smap, const char *key, bool val);
void smap_put_char(smap_t *smap, const char *key, char val);
void smap_put_byte(smap_t *smap, const char *key, byte val);
void smap_put_s16b(smap_t *smap, const char *key, s16b val);
void smap_put_u16b(smap_t *smap, const char *key, u16b val);
void smap_put_s32b(smap_t *smap, const char *key, s32b val);
void smap_put_u32b(smap_t *smap, const char *key, u32b val);
void smap_put_str(smap_t *smap,  const char *key, const char *val);
void smap_put_blob(smap_t *smap, const char *key, void *blob, u32b len);

bool smap_get_bool(smap_t *smap, const char *key);
char smap_get_char(smap_t *smap, const char *key);
byte smap_get_byte(smap_t *smap, const char *key);
s16b smap_get_s16b(smap_t *smap, const char *key);
u16b smap_get_u16b(smap_t *smap, const char *key);
s32b smap_get_s32b(smap_t *smap, const char *key);
u32b smap_get_u32b(smap_t *smap, const char *key);
const char *smap_get_str(smap_t *smap, const char *key);
const void *smap_get_blob(smap_t *smap, const char *key, u32b *len);

byte *smap_tostring(smap_t *smap, u32b *length);
smap_t *smap_fromstring(const byte *string, u32b length);

void smap_foreach(smap_t *smap, void (*fn)(sentry_t *se));
void smap_print(smap_t *smap);

#endif /* !INCLUDED_SERIALIZE_H */
