#ifndef UTILS_H
#define UTILS_H

unsigned long get_esp(void)
{
    unsigned long esp;
    __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
    return esp;
}

#endif /* UTILS_H */
