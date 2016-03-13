#include "cpu.h"
#include "error.h"
#include "ram.h"

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <ucontext.h>

// Globals
static void *g_vm_base = NULL;
static void *g_ram_base = NULL;
static unsigned int g_ram_size = 0;
static unsigned int g_vm_size = 0;
static struct sigaction g_old_segv_action = {0};
static struct sigaction g_old_trap_action = {0};
static ucontext_t g_ctx = {0};
static siginfo_t g_siginfo = {0};

void * cr3 = NULL;

// Private Function Declarations

void set_trace_flag(ucontext_t *ctx);
void clear_trace_flag(ucontext_t *ctx);
void save_pending_operation_state(ucontext_t *ctx, siginfo_t *info);
void clear_pending_operation_state();
int make_pending_vpages_accessible();
int restore_pending_vpages();
int do_read_operation();
int do_write_operation();
bool is_pending_read_operation();
bool is_pending_write_operation();

/*
 * cpu_vm_handler - triggers on access to a virtual memory address. does a page
 *                  walk to translate the address. if the page walk fails, raises
 *                  an error. otherwise, single steps the instruction, allowing it
 *                  to continue and upon re-entry, sets protections again.
 */
void cpu_vm_handler(int signum, siginfo_t *info, void *ctx);

int cpu_make_vm(unsigned int vm_size, void **vm_base);

int cpu_free_vm(void *vm_base, unsigned int vm_size);

int cpu_register_vm_handler();

int cpu_clear_vm_handler();

// Private Definitions
void clear_pending_operation_state() {
    memset(&g_ctx, '\0', sizeof(g_ctx));
    memset(&g_siginfo, '\0', sizeof(g_siginfo));
}

void set_trace_flag(ucontext_t *ctx) {
    ctx->uc_mcontext.gregs[REG_EFL] |= 0x100;
}

void clear_trace_flag(ucontext_t *ctx) {
    ctx->uc_mcontext.gregs[REG_EFL] &= ~0x100;
}

int do_read_operation() {
    void *phys_addr = v2p(g_siginfo.si_addr);
    if(phys_addr == NULL) {
        return ERR_SYS;
    }
    return ERR_SUCCESS;
}

int do_write_operation() {
    void *phys_addr = v2p(g_siginfo.si_addr);
    if(phys_addr == NULL) {
        return ERR_SYS;
    }
    return ERR_SUCCESS;
}

int make_pending_vpages_accessible() {
#define MAX_MEMOP_LEN (16)
    return mprotect(g_siginfo.si_addr, MAX_MEMOP_LEN, PROT_READ|PROT_WRITE);
}

int restore_pending_vpages() {
    return mprotect(g_siginfo.si_addr, MAX_MEMOP_LEN, PROT_NONE);
}

void save_pending_operation_state(ucontext_t *ctx, siginfo_t *info) {
    memcpy(&g_ctx, ctx, sizeof(*ctx));
    memcpy(&g_siginfo, info, sizeof(*info));
}

bool is_pending_read_operation() {
    return (g_ctx.uc_mcontext.gregs[REG_ERR] & 2) == 0;
}

bool is_pending_write_operation() {
    printf("reg_err: 0x%08x\n", (unsigned int)g_ctx.uc_mcontext.gregs[REG_ERR]);
    return g_ctx.uc_mcontext.gregs[REG_ERR] & 2;
}

int cpu_free_vm(void *vm_base, unsigned int vm_size) {
    if(munmap(vm_base, vm_size) < 0) 
        return ERR_SYS;

    return ERR_SUCCESS;
}

int cpu_make_vm(unsigned int vm_size, void **vm_base) {

    assert(vm_base != NULL);
    
    *vm_base = mmap(NULL, vm_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(*vm_base == MAP_FAILED)
        return ERR_SYS;

    return ERR_SUCCESS;
}

int cpu_clear_vm_handler() {
    int result = 0;

    result = sigaction(SIGSEGV, &g_old_segv_action, NULL);
    if(result < 0)
        return result;

    result = sigaction(SIGTRAP, &g_old_trap_action, NULL);
    if(result < 0)
        return result;

    return result;
}

int cpu_register_vm_handler() {
    int result = 0;

    struct sigaction newaction = {
        .sa_handler = NULL,
        .sa_sigaction = cpu_vm_handler,
        .sa_mask = 0,
        .sa_flags = SA_SIGINFO,
        .sa_restorer = NULL
    };

    result = sigaction(SIGSEGV, &newaction, &g_old_segv_action);
    if(result < 0)
        return result;

    result = sigaction(SIGTRAP, &newaction, &g_old_trap_action);
    if(result < 0)
        return result;

    return result;
}

void cpu_vm_handler(int signum, siginfo_t *info, void *ctx) {
    ucontext_t *u_ctx = (ucontext_t *)ctx;
    int err = 0;

    printf("Received %d signal\n", signum);
    if(signum == SIGSEGV) {
        void *v_addr = NULL;

        save_pending_operation_state(u_ctx, info);

        v_addr = info->si_addr;

        if((err = make_pending_vpages_accessible()) < 0) {
            fprintf(stderr, "Error: failed to make virtual pages writable\n");
        }

        if(is_pending_read_operation()) {
            if((err = do_read_operation()) < 0) {
                fprintf(stderr, "Error: failed while prepping read operation\n");
            }
        }

        set_trace_flag(u_ctx);
    } else if(signum == SIGTRAP) {

        if(is_pending_write_operation()) {
            if((err = do_write_operation()) < 0) {
                fprintf(stderr, "Error: failed while prepping write operation\n");
            }

            restore_pending_vpages();
        } else {
            fprintf(stderr, "Error: unexpected operation is pending\n");
        }

        clear_pending_operation_state();
        clear_trace_flag(u_ctx);
    } else {
        fprintf(stderr, "Warning: unexpected signal %d\n", signum);
    }
}

// Public Definitions
int cpu_init(unsigned int ram_size) {
    void *ram_base = NULL;
    int err = ERR_SUCCESS;

    ram_base = ram_init(ram_size);

    g_ram_base = ram_base;
    g_ram_size = ram_size;
    g_vm_size = g_ram_size;

    if((err = cpu_make_vm(g_vm_size, &g_vm_base) < 0))
        return err;

    if((err = cpu_register_vm_handler()) < 0)
        return err;

    return err;
}

int cpu_shutdown(void) {
    int err = ERR_SUCCESS;

    if((err = cpu_clear_vm_handler()) < 0)
        return err;

    if((err = cpu_free_vm(g_vm_base, g_vm_size)) < 0)
        return err;

    ram_clean();

    return err;
}

void *cpu_get_vm_base() {
    return g_vm_base;
}
