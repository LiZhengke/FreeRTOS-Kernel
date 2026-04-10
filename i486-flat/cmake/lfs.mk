include(FetchContent)

FetchContent_Declare(
  littlefs
  GIT_REPOSITORY https://github.com/littlefs-project/littlefs.git
  GIT_TAG        v2.11.0   # 建议固定版本！
)

FetchContent_MakeAvailable(littlefs)

add_library(littlefs STATIC
    ${littlefs_SOURCE_DIR}/lfs.c
    ${littlefs_SOURCE_DIR}/lfs_util.c
)

target_include_directories(littlefs PUBLIC
    ${littlefs_SOURCE_DIR}
)
