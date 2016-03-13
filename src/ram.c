#include "ram.h"

#include <stdlib.h>

void *g_ram_ptr = NULL;

void *ram_init(unsigned int ram_size) {
	g_ram_ptr = malloc(ram_size);
	return g_ram_ptr;
}

void ram_clean() {
	free(g_ram_ptr);
}
