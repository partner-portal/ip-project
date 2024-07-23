/*
 * Copyright (c) 2014-2023 ProvenRun S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * ProvenRun S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with ProvenRun S.A.S
 *
 * PROVENRUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVENRUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */
/**
 * @file
 * @brief ProvenCore syscall wrappers for in-house standard library.
 * @author Vincent Siles
 * @date April 14th, 2014 (creation)
 * @copyright (c) 2014-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBPNC_SYSCALL_H_INCLUDED_
#define _LIBPNC_SYSCALL_H_INCLUDED_

#include <compiler.h>
#include <errno.h>
#include <string.h> /* strlen() */
#include <sys/cdefs.h>          /* __BEGIN_DECLS */
#include <syscall_internal.h>
#include <unistd.h>

__BEGIN_DECLS

#if defined(CONFIG_ARCH_ARMV7A)
    #include "arch/armv7a/syscall_low.h"
#elif defined(CONFIG_ARCH_ARMV8)
    #include "arch/armv8/syscall_low.h"
#elif defined(CONFIG_ARCH_RISCV64)
    #include "arch/riscv64/syscall_low.h"
#else
    #error "Unsupported architecture"
#endif /* CONFIG_ARCH_* */

/**
 * @details
 *   usleep is not recommended for use by the standard.
 *   We define udelay as a replacement.
 * @uses SLEEP GETINFO
 */
int udelay(uint64_t microseconds);

/* Non-standard <unistd.h> function prototypes. */

/**
 * @uses GETINFO
 */
int get_pid(s_pid_t *pid);
/**
 * @uses SENDREC KILL
 */
int s_kill(s_pid_t target, uint32_t signal);
/**
 * @uses SENDREC EXEC BREAK
 */
int s_execve(const char *path, char *const argv[],
             char *const envp[], size_t quota);

/*
 * The standard-like prototype should `s_pid_t s_fork(void);` but \ref s_pid_t
 * being an unsigned type, error values cannot be returned without a cast.
 */
/**
 * @uses SENDREC FORK
 */
int s_fork(s_pid_t *pid);

/**
 * @uses SLEEP
 */
static inline int s_sleep(uint64_t timeout)
{
    return pnc_sleep(timeout);
}

/**
 * @uses IOMAP
 */
static inline int sys_iomap(unsigned int devid, paddr_t paddr,
                            size_t length, void **vaddr)
{
    return pnc_iomap(devid, paddr, length, vaddr);
}

/**
 * @uses IOMAP
 */
static inline int sys_devmap(unsigned int devid, void **vaddr)
{
    return pnc_iomap(devid, 0, 0, vaddr);
}

/**
 * @uses IOUNMAP
 */
static inline int sys_iounmap(void *vaddr)
{
    return pnc_iounmap(vaddr);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqctl(unsigned int request, unsigned int irq,
                             unsigned int policy, unsigned int id,
                             unsigned int status, unsigned int *hook_id)
{
    return pnc_irqctl(request, irq, policy, id, status, hook_id);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqstatus(unsigned int hook_id, unsigned int status)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, status, NULL);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqdisable(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, IRQ_DISABLED, NULL);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqenable(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, IRQ_ENABLED, NULL);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqrelease(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, IRQ_RELEASED, NULL);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqsetpolicy(unsigned int irq, unsigned int policy,
                                   unsigned int notify_id, unsigned int status,
                                   unsigned int *hook_id)
{
    return pnc_irqctl(IRQ_SETPOLICY, irq, policy, notify_id, status, hook_id);
}

/**
 * @uses IRQCTL
 */
static inline int sys_irqrmpolicy(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_RMPOLICY, 0, 0, hook_id, 0, NULL);
}

/**
 * @uses PLATCTL
 */
static inline int sys_platctl(unsigned int devid, unsigned int op,
                              message *payload)
{
    return pnc_platctl(devid, op, payload);
}

#if defined(CONFIG_ARCH_ARMV7A) || defined(CONFIG_ARCH_ARMV8)

/**
 * @uses SMC
 */
static inline int smc(smc32_params_t *params)
{
    return pnc_smc(params);
}

#ifdef CONFIG_ARCH_ARMV8

/**
 * @uses SMC
 */
static inline int smc64(smc64_params_t *params)
{
    return pnc_smc64(params);
}

#endif /* CONFIG_ARCH_ARMV8 */

/**
 * @uses HVC
 */
static inline int hvc(hvc32_params_t *params)
{
    return pnc_hvc(params);
}

#ifdef CONFIG_ARCH_ARMV8

static inline int hvc64(hvc64_params_t *params)
{
    return pnc_hvc64(params);
}

#endif /* CONFIG_ARCH_ARMV8 */

#endif /* defined(CONFIG_KARCH_ARMV7A) || defined(CONFIG_KARCH_ARMV8) */

#ifdef CONFIG_ARCH_RISCV64

/**
 * @uses SMC
 */
static inline int mcall(mcall_params_t *params)
{
    return pnc_mcall(params);
}

#endif /* CONFIG_ARCH_RISCV64 */

/**
 * @uses VIRCOPY
 */
static inline int vircopy(s_pid_t src, void const *src_vaddr, s_pid_t dst,
                          void *dst_vaddr, size_t length)
{
    return pnc_vircopy(src, src_vaddr, dst, dst_vaddr, length);
}

/**
 * @uses SETAUTH
 */
__access_none(3)
static inline int setauth(int mode, s_pid_t grantee, void const *vaddr,
                          size_t length, s_pid_t effector, s_pid_t revoker,
                          unsigned int *auth_handle)
{
    return pnc_setauth(mode, grantee, vaddr, length, effector, revoker,
                       auth_handle);
}

/**
 * @uses REVOKE
 */
static inline int revoke(s_pid_t owner, unsigned int auth_handle)
{
    return pnc_revoke(owner, auth_handle);
}

/**
 * @uses CHANGE_TARGET
 */
static inline int change_grantee(s_pid_t owner, unsigned int auth_handle,
                                 s_pid_t new_grantee)
{
    return pnc_change_grantee(owner, auth_handle, new_grantee);
}

/**
 * @uses CHANGE_EFFECTOR
 */
static inline int change_effector(s_pid_t owner, unsigned int auth_handle,
                                  s_pid_t new_effector)
{
    return pnc_change_effector(owner, auth_handle, new_effector);
}

/**
 * @uses CHANGE_REVOKER
 */
static inline int change_revoker(s_pid_t owner, unsigned int auth_handle,
                                 s_pid_t new_revoker)
{
    return pnc_change_revoker(owner, auth_handle, new_revoker);
}

/**
 * @uses SHM_ALLOC
 */
static inline int shm_alloc(unsigned int nr_pages, unsigned int *id,
                            void **addr)
{
    return pnc_shm_alloc(nr_pages, id, addr);
}

/**
 * @uses SHM_REGISTER
 */
static inline int shm_register(unsigned int id, unsigned int *nr_pages,
                               void **vaddr)
{
    return pnc_shm_register(id, nr_pages, vaddr);
}

/**
 * @uses SHM_UNREGISTER
 */
static inline int shm_unregister(unsigned int id)
{
    return pnc_shm_unregister(id);
}

/**
 * @uses SHM_TRANSFER
 */
static inline int shm_transfer(unsigned int id, s_pid_t target)
{
    return pnc_shm_transfer(id, target);
}

/**
 * @uses GETINFO
 */
static inline int get_domain_of(s_pid_t object, unsigned int *dom)
{
    return pnc_get_domain(object, dom);
}

/**
 * @uses GETINFO
 */
static inline int get_domain(unsigned int *dom)
{
    s_pid_t self;
    get_pid(&self);
    return pnc_get_domain(self, dom);
}

/**
 * @uses GETINFO
 */
static inline int get_times_of(s_pid_t object, uint64_t *user_time)
{
    return pnc_get_times(object, user_time);
}

/**
 * @uses GETINFO
 */
static inline int get_times(uint64_t *user_time)
{
    s_pid_t self;
    get_pid(&self);
    return pnc_get_times(self, user_time);
}

/**
 * @uses GETINFO
 */
static inline int get_uptime(uint64_t *uptime)
{
    return pnc_get_uptime(uptime);
}

/**
 * @uses GETINFO
 */
static inline int get_quota_of(s_pid_t object,
                               unsigned int *quota, unsigned int *max_quota)
{
    return pnc_get_quota(object, quota, max_quota);
}

/**
 * @uses GETINFO
 */
static inline int get_quota(unsigned int *quota, unsigned int *max_quota)
{
    s_pid_t self;
    get_pid(&self);
    return pnc_get_quota(self, quota, max_quota);
}

/**
 * @uses GETINFO
 */
static inline int get_data_of(s_pid_t object,
                              void **pdata_base, unsigned int *data_size)
{
    return pnc_get_data(object, pdata_base, data_size);
}

/**
 * @uses GETINFO
 */
static inline int get_data(void **pdata_base, unsigned int *data_size)
{
    s_pid_t self;
    get_pid(&self);
    return pnc_get_data(self, pdata_base, data_size);
}

/**
 * @uses GETINFO
 */
static inline int get_metadata_by_app(char const *elf_name, size_t length,
                                      uint32_t *data_size,
                                      uint32_t *minfo, uint32_t *elf_nr)
{
    return pnc_get_metadata_by_app(elf_name, length, data_size, minfo, elf_nr);
}

/**
 * @uses GETINFO
 */
static inline int get_metadata_by_pid(s_pid_t id, uint32_t *data_size,
                                      uint32_t *minfo, uint32_t *elf_nr)
{
    return pnc_get_metadata_by_pid(id, data_size, minfo, elf_nr);
}

/**
 * @uses GETINFO
 */
static inline int get_shm_token_owner(unsigned int id, s_pid_t *owner)
{
    return pnc_get_shm_token_owner(id, owner);
}

/**
 * @uses GETINFO
 */
static inline int get_phys_buff(void **vaddr, paddr_t *paddr,
                                unsigned int *nr_pages)
{
    return pnc_get_phys_buff(vaddr, paddr, nr_pages);
}

/**
 * @uses GETINFO
 */
static inline int get_sysproc_pid_by_name(char const *sysproc_name, size_t length,
                                          s_pid_t *pid)
{
    return pnc_get_sysproc_pid_by_name(sysproc_name, length, pid);
}

/**
 * @uses GETINFO
 */
static inline int get_capabilities_of(s_pid_t object,
                                      uint32_t *priv_flags, uint32_t *trap_mask,
                                      uint64_t *call_mask)
{
    return pnc_get_capabilities(object, priv_flags, trap_mask, call_mask);
}

/**
 * @uses GETINFO
 */
static inline int get_capabilities(uint32_t *priv_flags, uint32_t *trap_mask,
                                   uint64_t *call_mask)
{
    s_pid_t self;
    get_pid(&self);
    return pnc_get_capabilities(self, priv_flags, trap_mask, call_mask);
}

/**
 * @uses GETINFO
 */
static inline int get_allowed_ipc_of(s_pid_t object, s_pid_t target, uint32_t *trap_mask)
{
    return pnc_get_allowed_ipc(object, target, trap_mask);
}

/**
 * @uses GETINFO
 */
static inline int get_allowed_ipc(s_pid_t target, uint32_t *trap_mask)
{
    s_pid_t self;
    get_pid(&self);
    return pnc_get_allowed_ipc(self, target, trap_mask);
}

/* Debug features. */

/**
 * @uses GETINFO
 */
static inline int check_memory_rights(unsigned int flags, void *vaddr,
                                      size_t length)
{
    return _pnc_check_memory_rights(flags, vaddr, length);
}

/**
 * @uses SENDREC EXEC
 */
static inline int load_exec(void *elf, size_t elf_len, char const *name,
                            unsigned int name_len)
{
    return _pnc_load_exec(elf, elf_len, name, name_len);
}

/**
 * @uses GETINFO
 */
static inline int dump_(unsigned int arg)
{
    return _pnc_dump(arg);
}

/**
 * @uses GETINFO
 */
static inline int get_free_pages(unsigned int *nr_pages)
{
    return _pnc_get_free_pages(nr_pages);
}

/* Other wrappers. */

/**
 * @uses GETINFO
 */
static inline int get_phys_buff_nrpages(s_pid_t id, unsigned int *nr_pages)
{
    uint32_t minfo;
    int ret = get_metadata_by_pid(id, NULL, &minfo, NULL);
    if (ret != OK)
        return ret;

    if (nr_pages)
        *nr_pages = METADATA_PHYSINFO_NRPAGES(minfo);
    return OK;
}

/**
 * @uses GETINFO
 */
static inline int sys_times(uint64_t *user_time)
{
    errno = get_times(user_time);
    return errno;
}

/**
 * @uses GETINFO
 */
static inline int uptime(uint64_t *uptime)
{
    errno = get_uptime(uptime);
    return errno;
}

/**
 * @uses BREAK
 */
static inline message *alloc_message(void)
{
    /*
     * syscall.h should not include <stdlib.h> which is from the 'higher'
     * level libc, so we declare the prototype of aligned_alloc here.
     * This will not pose a problem as the function is static inline
     * (no linkage dependency).
     */
    void *aligned_alloc(size_t alignment, size_t size);
    /*
     * The message alignment has been chosen to avoid crossing a page boundary.
     * See the comments for message type declaration.
     */
    return (message *) aligned_alloc(_Alignof(message), sizeof(message));
}

/**
 * @brief Wrapper around get_metadata_by_app
 *
 * This function expects \p elf_name to be a NUL-terminated string
 * @uses GETINFO
 */
static inline int get_metadata_by_app_0(char const *elf_name,
                                        uint32_t *data_size,
                                        uint32_t *minfo, uint32_t *elf_nr)
{
    return get_metadata_by_app(elf_name, strlen(elf_name),
                               data_size, minfo, elf_nr);
}

/* Issuing a system call has a cost, thus we define a dedicated structure
 * to put in cache the recovered sysprocs, rather than issuing a call
 * to get_sysproc_pid_by_name() each time.
 *
 * Variable of this type should always be declared and defined as in:
 * (static) cached_pid_t <variable> = CACHE_PID("<name>");
 *
 * Fields should not be accessed directly, and this structure is
 * only meant to be passed to get_sysproc_pid().
 */
typedef struct {
    s_pid_t sysproc_pid;
    const char *sysproc_name;
    unsigned int sysproc_searched : 1;
} cached_pid_t;
#define CACHE_PID(name) { .sysproc_name = name, .sysproc_searched = 0 }

/**
 * @uses GETINFO
 */
static inline s_pid_t get_sysproc_pid(cached_pid_t *sp)
{
    if (sp == NULL)
        return INVALID_PID(EFAULT);
    if (!sp->sysproc_searched) {
        int r = get_sysproc_pid_by_name(sp->sysproc_name,
                                        strlen(sp->sysproc_name),
                                        &sp->sysproc_pid);
        if (r != OK)
            sp->sysproc_pid = INVALID_PID(r);
        sp->sysproc_searched = 1;
    }
    return sp->sysproc_pid;
}

__END_DECLS

#endif /* _LIBPNC_SYSCALL_H_INCLUDED_ */
