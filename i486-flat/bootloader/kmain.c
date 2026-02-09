void kmain(void) {
    /* 1. Initialize serial port...

    // 2. Load interrupt descriptor table
    // init_idt();
    serial_print("IDT initialized.\n");

    // 3. Deliberately cause a divide by zero exception (Exception 0)
    serial_print("Testing exception 0...\n");
    int a = 10;
    int b = 0;
    int c = a / b;  // This will trigger exc0 -> exception_handler

    while(1);*/
    serial_print("Start kernel...\n");
}
