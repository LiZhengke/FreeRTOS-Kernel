include(FetchContent)

FetchContent_Declare(
  fatfs
  GIT_REPOSITORY https://github.com/abbrev/fatfs.git
  GIT_TAG        R0.16
)

FetchContent_Populate(fatfs)

# Override the bundled ffconf.h with the project-specific one.
configure_file(
    ${CMAKE_SOURCE_DIR}/../fs/fatfs/ffconf.h
    ${fatfs_SOURCE_DIR}/source/ffconf.h
    COPYONLY
)

add_library(fatfs STATIC
    ${fatfs_SOURCE_DIR}/source/ff.c
    ${fatfs_SOURCE_DIR}/source/ffsystem.c
    ${fatfs_SOURCE_DIR}/source/ffunicode.c
    ${CMAKE_SOURCE_DIR}/../fs/fatfs/diskio.c
    ${CMAKE_SOURCE_DIR}/../fs/fatfs/fatfs_test.c
    ${CMAKE_SOURCE_DIR}/../../blk/ata.c
    ${CMAKE_SOURCE_DIR}/../../portable/GCC/I486_flat/os_helper.c
)

target_include_directories(fatfs PUBLIC
    ${CMAKE_SOURCE_DIR}/../fs/fatfs
    ${CMAKE_SOURCE_DIR}/../../blk
    ${CMAKE_SOURCE_DIR}/../../include
    ${CMAKE_SOURCE_DIR}/../../portable/GCC/I486_flat
    PRIVATE
    ${fatfs_SOURCE_DIR}/source
)

target_compile_options(fatfs PRIVATE
    -Wno-unused-parameter
    -Wno-unused-variable
)
