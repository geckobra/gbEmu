#include <stdio.h>
#include "CPU.h"
#include "MMU.h"

int main(int argc, char* argv[]){
    printf("Running Gameboy Emulator!\n");

    if (argc < 2){
        printf("No ROM provided, terminating\n");
        return 0;
    }

    mem_init();
    cpu_init();

    printf("Loading ROM at %s\n", argv[1]);
    if (load_ROM(argv[1]) < 0){
        printf("Couldn't read ROM!!\n");
    }
    
    while (1){
        run_cpu();
    }

    return 0;
}