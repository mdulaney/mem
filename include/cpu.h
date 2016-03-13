#include "cpu_shared.h"

unsigned int cpu_get_cr3();

void cpu_set_cr3(unsigned int val);

void cpu_raise_fault(int fault);

void cpu_disable_vmem();

void cpu_enable_vmem();

void *cpu_get_vm_base();

int cpu_init(unsigned int ram_size);

int cpu_shutdown(void);
