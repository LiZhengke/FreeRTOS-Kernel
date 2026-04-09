cmake_minimum_required(VERSION 3.10)
set(projectName "i486-flat-application")
project(${projectName} LANGUAGES C ASM)

add_executable(${projectName}
	src/start.S
    src/user_task.c
    src/io.c
)
target_include_directories(${projectName} PRIVATE inc)
target_link_options(${projectName}
	PRIVATE
	-T ${CMAKE_CURRENT_SOURCE_DIR}/user.ld
	-Wl,-Map=${projectName}.map
	-Wl,--cref
)

set(USER_TASK_MAIN_OBJ ${CMAKE_CURRENT_BINARY_DIR}/user_task_main.o)
add_custom_command(
	OUTPUT ${USER_TASK_MAIN_OBJ}
	COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-i386 ${projectName} ${USER_TASK_MAIN_OBJ}
	DEPENDS ${projectName}
	COMMENT "Assembling user task main object file"
	VERBATIM
)

add_custom_target(user_task_main ALL DEPENDS ${USER_TASK_MAIN_OBJ})
