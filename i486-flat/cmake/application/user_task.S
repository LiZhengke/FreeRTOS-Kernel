.code32
.text
.global _start
.type _start, @function

_start:
    /* 此时我们已经在 Ring 3 了 */
    mov $0, %eax          # 初始化计数器
.loop:
    inc %eax             # 增加计数器

    /* 如果你想测试系统调用，取消下面两行的注释 */
    /* mov %ebx, %eax      # 把值传给 ebx 作为参数 */
    /* int 0x80          # 触发系统调用中断 */

    jmp .loop           # 死循环
