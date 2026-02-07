# ----------------------------------------
# i486-flat FreeRTOS toolchain (32-bit)
# ----------------------------------------

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i486)

# host gcc（can be i686-elf-gcc）
set(CMAKE_C_COMPILER gcc)
set(CMAKE_ASM_COMPILER gcc)
set(CMAKE_LINKER ld)

# Force 32-bit + freestanding
set(COMMON_FLAGS "-m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -fno-builtin -Wall -Wextra -Werror")

set(CMAKE_C_FLAGS "${COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS "-m32 -fno-pie -fno-pic")
set(CMAKE_EXE_LINKER_FLAGS "-m32 -nostdlib -ffreestanding -no-pie -Wl,--build-id=none")

# Stop CMake to run any test
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
