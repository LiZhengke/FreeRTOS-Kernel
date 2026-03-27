#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

.code32

# 建议使用专门的段名，方便在链接脚本中精确控制
.section .user_bin, "a"

# 极其重要：确保二进制块在内核镜像中是页对齐的（4096 字节）
# 这样你 memcpy 时，源地址和目的地址都是 0x...000 对齐，不会出错
.align 4096

.incbin STR(USER_BIN_PATH)

# 结尾也对齐，确保计算出来的 size 是 4KB 的整数倍（可选，但推荐）
.align 4096
