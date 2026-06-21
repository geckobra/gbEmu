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
uint8_t fetch(){
    return read_memory(cpu_registers.pc++);
}

void run_cpu(){
    int t_cycles_executed = 0;

    //if the cpu is halted, don't fetch next instruction
    if (isHalted){
        t_cycles_executed = 4;
    } else{
        next_instruction = fetch();
        t_cycles_executed = execute(next_instruction);
    }
    
    total_t_cycles += t_cycles_executed;
}

uint8_t alu_add8(uint8_t val){
    //performs addition to the accumulator register and sets corresponding flags

    uint8_t flags_mask = 0x0; //create empty flag mask (Z, N, H, C)
    uint8_t a_val = cpu_registers.a;

    uint16_t result = (uint16_t)a_val + (uint16_t)val;

    if ((uint8_t)result == 0)                 flags_mask |= 1 << 7; //set zero flag
    if ((a_val & 0x0F) + (val & 0x0F) > 0x0F) flags_mask |= 1 << 5; //set half-carry flag
    if (result > 0xFF)                        flags_mask |= 1 << 4; //set carry flag
    
    cpu_registers.f = flags_mask; //set and reset all the flags

    return (uint8_t)result;
}

uint8_t alu_adc8(uint8_t val){
    //performs addition with carry to the accumulator register and sets corresponding flags

    uint8_t flags_mask = 0x0;
    uint8_t a_val = cpu_registers.a;

    uint8_t carry_flag = cpu_registers.a & 1<<4;

    uint16_t result = (uint16_t)a_val + (uint16_t)val + carry_flag;

    if ((uint8_t)result == 0)                               flags_mask |= 1 << 7; //set zero flag
    if ((a_val & 0x0F) + (val & 0x0F) + carry_flag > 0x0F)  flags_mask |= 1 << 5; //set half-carry flag
    if (result > 0xFF)                                      flags_mask |= 1 << 4; //set carry flag

    return (uint8_t)result;
} 