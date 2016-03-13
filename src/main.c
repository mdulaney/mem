#include <stdio.h>

#include "cpu.h"
#include "mem_manager.h"
#include "ram.h"

void hw_initialize() {

    // initialize the RAM
    ram_init();

    // initialize the semi-virtualized CPU
    cpu_init(ram_get_base(), ram_get_size());

}

int main(int argc, char *argv[]) {

    printf("*** Simple MMU Simulator ***\n");

    printf("Performing hardware initialization\n");
    hw_initialize();

    printf("Performing memory manager initialization\n");
    mem_manager_init();

    printf("Cleaning up the memory manager\n");
    mem_manager_exit();
    return 0;
}
