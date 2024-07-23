# This file lists kernel and applications/libraries options

# WARNING: Kernel features are provided for informations only,
#          as the kernel is pre-built. You must NOT modify them.
#          These features are passed to applications/libraries.
kernel_FEATURES := TRUSTZONE NO_SECURE_MONITOR SKIP_CONSOLE_INIT YIELD_TO_NON_SECURE NS_64 S_64 GICV3 PNC_CORE=1 MAIN_BANNER SINGLE_BINARY_IMAGE KERNEL_UART_COLOR VERBOSE=PR_ERR_VERBOSE K_MAIN_VERBOSE=PR_ERR_VERBOSE K_TZ_VERBOSE=PR_ERR_VERBOSE ERR_DUMP_REGS ERR_DUMP_STACK ERR_BACKTRACE K_FAULT_VERBOSE=PR_ERR_VERBOSE K_MMAP_VERBOSE=PR_ERR_VERBOSE K_IRQ_VERBOSE=PR_ERR_VERBOSE K_NXP_XRDC_VERBOSE=PR_WARN_VERBOSE K_S32GVNPRDB3_SEC_VERBOSE=PR_WARN_VERBOSE

# Below are options related to applications and libraries.
#  You may modify them as you wish.
# WARNING: keep in mind changing them will not change pre-built
#          applications. Do not introduce option clashes.
user_FEATURES := REEV3 \
    REE_TESTS \
    REBOOT_REE \
    REE_VERBOSE=PR_ERR_VERBOSE \
    TOUPPER_VERBOSE=PR_ERR_VERBOSE \
    SKIP_CONSOLE_INIT \
    UART_COLOR \
    SOCKET_LWIP \
    ONLY_STATIC_ENTRY \
    ETH_DISPLAY_PACKET \
    EPH_RANGE_LINUX \
    

app_FEATURES := 
PNC_LIBC ?= libstd
