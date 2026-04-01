#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

.code32

# Use a dedicated section name to allow precise control in the linker script.
.section .user_bin, "a"

# Very important: ensure the binary blob is page-aligned (4096 bytes) in the kernel image.
# This keeps both source and destination addresses 0x...000 aligned during memcpy.
.align 4096

.incbin STR(USER_BIN_PATH)

# Align the end as well so computed size is a multiple of 4KB (optional but recommended).
.align 4096
