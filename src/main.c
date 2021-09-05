#include "../include/vm.h"

static uint32_t program[] = {
    ICONST, 3,
    ICONST, 12,
    IADD, 
    SYS_WRITE,
    HALT
};

int main() {
    init_vm();

    run_vm(0, program, 0);

    return 0;
}