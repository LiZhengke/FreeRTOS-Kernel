include(FetchContent)

FetchContent_Declare(
  fatfs
  GIT_REPOSITORY https://github.com/abbrev/fatfs.git
  GIT_TAG        R0.16
)

FetchContent_Populate(fatfs)

add_library(fatfs STATIC
    ${fatfs_SOURCE_DIR}/source/ff.c
    ${fatfs_SOURCE_DIR}/source/ffsystem.c
    ${fatfs_SOURCE_DIR}/source/ffunicode.c
    ${CMAKE_SOURCE_DIR}/../fs/fatfs/diskio.c
)

target_include_directories(fatfs PUBLIC
    ${CMAKE_SOURCE_DIR}/../fs/fatfs
    ${CMAKE_SOURCE_DIR}/../../blk
    ${CMAKE_SOURCE_DIR}/../../include
    PRIVATE
    ${fatfs_SOURCE_DIR}/source
)
