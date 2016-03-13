#include "cpu_shared.h"

#include <assert.h>
#include <stdlib.h>

extern void *cr3;

inline void *v2p(void *v) {
	assert(cr3);
	return NULL;
}
