#include "CPU.h"
#include "MMU.h"
#include <string.h>

sm83_registers_t cpu_registers = {};
bool isHalted = false;

uint8_t next_instruction = 0x0; //next instruction to be executed
uint64_t total_t_cycles = 0;

void cpu_init(){
    memset(&cpu_registers, 0, sizeof(cpu_registers));

    cpu_registers.pc = 0xC000;
}

//fetches the next instruction from WRAM memory
uint8_t fetch(uint16_t pc){
    return read_memory(cpu_registers.pc++);
}

void run_cpu(){
    int t_cycles_executed = 0;

    //if the cpu is halted, don't fetch next instruction
    if (!isHalted){
        t_cycles_executed = 4;
    } else{
        next_instruction = fetch(cpu_registers.pc);
        t_cycles_executed = execute(next_instruction);
    }
    
    total_t_cycles += t_cycles_executed;
}