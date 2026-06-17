#include "CPU.h"
#include "MMU.h"
#include <string.h>

sm83_registers_t cpu_registers;

uint8_t next_instruction = 0x0; //next instruction to be executed
uint64_t total_t_cycles = 0;

void cpu_init(){
    memset(&cpu_registers, 0, sizeof(cpu_registers));

    cpu_registers.pc = 0xC000;
}

//fetches the next instruction from WRAM memory
static uint8_t fetch(uint16_t pc){
    cpu_registers.pc++; //move to next byte
    return read_memory(cpu_registers.pc);
}

//returns the T-Cycles needed to run the instruction and advances the PC as needed
static int execute(uint8_t opcode){

    int executed_t_ticks = 0;

    switch(opcode){
        case 0x0:
            //NOP, doesn't do anything
            executed_t_ticks = 4;
        break;

        case 0x01:
            //load n16 value into BC register
            uint8_t val_h = fetch(cpu_registers.pc++);
            uint8_t val_l = fetch(cpu_registers.pc++);

            cpu_registers.bc = ((uint16_t)val_h<<8) | val_l; //load the value into the bc register

            executed_t_ticks = 12;

            break;

        case 0x02:
            //load the value stored in A at the memory pointed to by BC
            write_memory(cpu_registers.bc, cpu_registers.a);

            executed_t_ticks = 8;

            break;

        case 0x06:
            uint8_t value = read_memory(cpu_registers.pc++);
            printf("Load %d into B\n", value);

            cpu_registers.b = value;
            
            executed_t_ticks = 8;
            break;

        case 0x0A:
            //load in regA the value stored at WRAM[BC]
            cpu_registers.a = read_memory(cpu_registers.bc);
            executed_t_ticks = 8;
            break;

        case 0x0E:
            //load an 8 bit value in regC
            uint8_t value = read_memory(cpu_registers.pc++);
            cpu_registers.c = value;
            executed_t_ticks = 8;
            break;

        case 0x12:
            //load in WRAM[DE] the value stored at regA
            write_memory(cpu_registers.de, cpu_registers.a);
            executed_t_ticks = 8;
            break;
    }

    return executed_t_ticks;
}

void run_cpu(){
    next_instruction = fetch(cpu_registers.pc);
    int t_cycles_executed = execute(next_instruction);

    total_t_cycles += t_cycles_executed;
}