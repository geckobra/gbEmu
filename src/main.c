#include <stdio.h>
#include "CPU.h"
#include "MMU.h"

int main(){
    printf("Hello, world!\n");

    cpu_init();
    write_memory(0xC000, 0x06);
    write_memory(0xC001, 200);

    while (1){
        run_cpu();
    }

    return 0;
}