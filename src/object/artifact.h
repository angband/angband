/* object/artifact.h */

#ifndef OBJECT_ARTIFACT_H
#define OBJECT_ARTIFACT_H

#include "object/object.h"

struct artifacts;

extern struct artifacts *artifacts_new();
extern void artifacts_add(struct artifacts *as, struct artifact *a);
extern struct artifact *artifacts_get(struct artifacts *as, size_t idx);

extern struct artifacts *artifacts;

#endif /* !OBJECT_ARTIFACT_H */
