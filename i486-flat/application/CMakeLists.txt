set(USER_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(USER_START_SRC "${USER_APP_DIR}/start.S")
set(USER_IO_SRC "${USER_APP_DIR}/io.c")
set(USER_TASK_SRC "${USER_APP_DIR}/user_task.c")
set(USER_LINKER_SCRIPT "${USER_APP_DIR}/user.ld")

set(USER_START_OBJ "${CMAKE_CURRENT_BINARY_DIR}/user_start.o")
set(USER_IO_OBJ "${CMAKE_CURRENT_BINARY_DIR}/io.o")
set(USER_TASK_OBJ "${CMAKE_CURRENT_BINARY_DIR}/user_task.o")
set(USER_TASK_ELF "${CMAKE_CURRENT_BINARY_DIR}/user_task.elf")
set(USER_BIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/user_task.bin")

set(USER_CFLAGS
	-m32
	-march=i486
	-ffreestanding
	-fno-pic
	-fno-pie
	-fno-stack-protector
	-fno-builtin
	-Os)

add_custom_command(
	OUTPUT ${USER_START_OBJ}
	COMMAND ${CMAKE_ASM_COMPILER} -m32 -march=i486 -fno-pie -fno-pic -c ${USER_START_SRC} -o ${USER_START_OBJ}
	DEPENDS ${USER_START_SRC}
	COMMENT "Assembling user start.S"
	VERBATIM
)

add_custom_command(
	OUTPUT ${USER_IO_OBJ}
	COMMAND ${CMAKE_C_COMPILER} ${USER_CFLAGS} -c ${USER_IO_SRC} -o ${USER_IO_OBJ}
	DEPENDS ${USER_IO_SRC}
	COMMENT "Compiling io.c"
	VERBATIM
)

add_custom_command(
	OUTPUT ${USER_TASK_OBJ}
	COMMAND ${CMAKE_C_COMPILER} ${USER_CFLAGS} -c ${USER_TASK_SRC} -o ${USER_TASK_OBJ}
	DEPENDS ${USER_TASK_SRC}
	COMMENT "Compiling user_task.c"
	VERBATIM
)

add_custom_command(
	OUTPUT ${USER_TASK_ELF}
	COMMAND ${CMAKE_C_COMPILER} -m32 -nostdlib -no-pie -Wl,--build-id=none -T ${USER_LINKER_SCRIPT} ${USER_START_OBJ} ${USER_IO_OBJ} ${USER_TASK_OBJ} -o ${USER_TASK_ELF}
	DEPENDS ${USER_START_OBJ} ${USER_IO_OBJ} ${USER_TASK_OBJ} ${USER_LINKER_SCRIPT}
	COMMENT "Linking user_task.elf"
	VERBATIM
)

add_custom_command(
	OUTPUT ${USER_BIN_FILE}
	COMMAND ${CMAKE_OBJCOPY} -O binary ${USER_TASK_ELF} ${USER_BIN_FILE}
	DEPENDS ${USER_TASK_ELF}
	COMMENT "Generating user_task.bin"
	VERBATIM
)

add_custom_target(user_task_bin DEPENDS ${USER_BIN_FILE})

set(USER_BIN_FILE "${USER_BIN_FILE}" PARENT_SCOPE)
set(USER_TASK_BIN_TARGET user_task_bin PARENT_SCOPE)

message(STATUS "USER_BIN_FILE: ${USER_BIN_FILE}")
