#include <stdint.h>

// IDT entry structure
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

// Serial port print function (assumed implemented)
void serial_print(const char* s);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

/*void init_idt() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // These entry addresses will be defined later in assembly
    extern void exc0(); // Divide by zero exception
    extern void exc13(); // General Protection Fault (GPF)

    idt_set_gate(0, (uint32_t)exc0, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)exc13, 0x08, 0x8E);

    __asm__ volatile("lidt %0" : : "m"(idtp));
}*/

/* kernel.c */

// Define serial port base address
#define COM1 0x3F8

// Simple port output function
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Re-wrap serial output in C
void putc(char c) {
    // Assume the serial port has been initialized in the assembly stage
    outb(COM1, c);
}

void serial_print(const char* s) {
    while(*s) putc(*s++);
}

/*void kmain(void) {
    print("Successfully entered C environment!\r\n");

    // Big things to do next:
    // 1. Detect and initialize PCI bus
    // 2. Initialize real DRAM (memory controller training)
    // 3. Copy itself from Flash ROM to RAM and run (Relocation)

    while(1);
}*/

/* kernel.c */

// Simple hexadecimal to string conversion and serial output
void print_hex(uint32_t n) {
    char *hex = "0123456789ABCDEF";
    serial_print("0x");
    for (int i = 28; i >= 0; i -= 4) {
        putc(hex[(n >> i) & 0xF]);
    }
}

// Enhanced exception handler
void exception_handler(uint32_t vector, uint32_t err_code) {
    serial_print("\n*** KERNEL PANIC ***\n");
    serial_print("Exception Vector: ");
    print_hex(vector);
    serial_print("\nError Code:      ");
    print_hex(err_code);
    serial_print("\nSystem Halted.\n");

    // If you also pass the pushal values from assembly, you can print all registers here
    while(1);
}
