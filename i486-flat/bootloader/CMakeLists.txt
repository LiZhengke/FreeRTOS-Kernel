cmake_minimum_required(VERSION 3.13)

project(i486-bootloader ASM C)

# Enable assembly language support
enable_language(ASM)

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --32")

# Source files
set(ASM_SOURCES
    reset.S
    start16.S
    start32.S
)

set(C_SOURCES
    kmain.c
    kernel.c
)

# Create object files for assembly sources
foreach(ASM_SRC ${ASM_SOURCES})
    get_filename_component(ASM_NAME ${ASM_SRC} NAME_WE)
    set(ASM_OBJ ${CMAKE_CURRENT_BINARY_DIR}/${ASM_NAME}.o)

    if(${ASM_SRC} STREQUAL "reset.S")
        add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND as --32 ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_SRC} -o ${ASM_OBJ}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_SRC}
            COMMENT "Assembling ${ASM_SRC}"
        )
    else()
        add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND gcc -m32 -c ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_SRC} -o ${ASM_OBJ}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_SRC}
            COMMENT "Assembling ${ASM_SRC} with gcc"
        )
    endif()

    list(APPEND ASM_OBJECTS ${ASM_OBJ})
endforeach()

# Create object files for C sources
foreach(C_SRC ${C_SOURCES})
    get_filename_component(C_NAME ${C_SRC} NAME_WE)
    set(C_OBJ ${CMAKE_CURRENT_BINARY_DIR}/${C_NAME}.o)

    add_custom_command(
        OUTPUT ${C_OBJ}
        COMMAND gcc -m32 -c ${CMAKE_CURRENT_SOURCE_DIR}/${C_SRC} -o ${C_OBJ}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${C_SRC}
        COMMENT "Compiling ${C_SRC}"
    )

    list(APPEND C_OBJECTS ${C_OBJ})
endforeach()

# Link all objects into ELF binary
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bios.elf
    COMMAND ld -m elf_i386
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        -nostdlib
        ${ASM_OBJECTS}
        ${C_OBJECTS}
        -o ${CMAKE_CURRENT_BINARY_DIR}/bios.elf
    DEPENDS ${ASM_OBJECTS} ${C_OBJECTS} ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
    COMMENT "Linking bios.elf"
)

# Convert ELF to binary ROM format
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bios.rom
    COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/bios.elf ${CMAKE_CURRENT_BINARY_DIR}/bios.rom
    COMMAND truncate -s 1048576 ${CMAKE_CURRENT_BINARY_DIR}/bios.rom
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bios.elf
    COMMENT "Creating bios.rom (1MB binary)"
)

# Custom target to build the bootloader
add_custom_target(bootloader ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bios.rom
)
