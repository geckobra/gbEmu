#include "CPU.h"
#include "MMU.h"
#include <string.h>

sm83_registers_t cpu_registers = {};
bool isHalted = false;
bool interrupts_enabled = false;
uint8_t ei_delay = 2;

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

    if (ei_delay > 0){
        ei_delay--;
        if (ei_delay == 0){
            interrupts_enabled = true;
        }
    }

    //if the cpu is halted, don't fetch next instruction
    if (isHalted){
        t_cycles_executed = 4;
    } else{
        next_instruction = fetch();
        t_cycles_executed = execute(next_instruction);
    }
    
    total_t_cycles += t_cycles_executed;
}


//ALU HELPER FUNCTIONS
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

    uint8_t carry_flag = (cpu_registers.f & (1<<4)) ? 1 : 0;

    uint16_t result = (uint16_t)a_val + (uint16_t)val + carry_flag;

    if ((uint8_t)result == 0)                               flags_mask |= 1 << 7; //set zero flag
    if ((a_val & 0x0F) + (val & 0x0F) + carry_flag > 0x0F)  flags_mask |= 1 << 5; //set half-carry flag
    if (result > 0xFF)                                      flags_mask |= 1 << 4; //set carry flag

    cpu_registers.f = flags_mask;
    return (uint8_t)result;
}

uint8_t alu_sub8(uint8_t val){
    //performs substraction to the accumulation register and sets corresopnding flags

    uint8_t flags_mask = 0x0;
    uint8_t a_val = cpu_registers.a;

    uint8_t result = a_val - val;

    flags_mask |= 1 << 6; //set N flag
    if (result == 0) flags_mask |= 1 << 7; //set Z flag
    if (((a_val & 0x0F) - (val & 0x0F)) & 0x10) flags_mask |= 1 << 5; //set H-carry flag
    if (val > a_val) flags_mask |= 1 << 4; //if value is higher than regA set Carry flag

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t alu_subc8(uint8_t val){

    uint8_t flags_mask = 0x0;

    uint8_t a_val = cpu_registers.a;
    uint8_t carry_flag = (cpu_registers.f & (1 << 4)) ? 1 : 0;

    uint8_t result = a_val - val - carry_flag;

    flags_mask |= 1 << 6; //set N flag
    if (result == 0) flags_mask |= 1 << 7;
    if (((a_val & 0x0F) - (val & 0x0F) - carry_flag) & 0x10) flags_mask |= 1 << 5; //set H-carry flag
    if ((int)a_val - (int)val - (int)carry_flag < 0) flags_mask |= 1 << 4; //borrow if the total value to be substracted is greater than regA

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t inc_r8(uint8_t reg){

    uint8_t flags_mask = 0x0;
    uint8_t result = reg+1;

    if (result == 0) flags_mask |= 1 << 7;
    if ((result & 0x0F) == 0x0F) flags_mask |= 1 << 5;

    flags_mask |= (cpu_registers.f & (1 << 4)); //carry flag shouldn't be affected by the operation

    cpu_registers.f = flags_mask;
    
    return result;
}

uint8_t dec_r8(uint8_t reg){
    //decrement reg by one and set flags accordingly

    uint8_t flags_mask = 0x0;
    uint8_t result = reg - 1;

    flags_mask |= (cpu_registers.f & 0x10); //keep contents of carry flag
    flags_mask |= (1<<6); //set N flag

    if (result == 0) flags_mask |= (1<<7);
    if ((reg & 0x0F) == 0) flags_mask |= (1<<5); //set H flag if there is borrow from bit 3 to bit 4

    cpu_registers.f = flags_mask;

    return result;
}

uint16_t alu_add16(uint16_t reg1, uint16_t reg2){

    uint8_t flags_mask = 0x0;
    uint32_t result = reg1+reg2;

    flags_mask |= (cpu_registers.f & 0x80); //keep contents of Zero flag

    if ((reg1 & 0x0FFF) + (reg2 & 0x0fFF) > 0x0FFF) flags_mask |= 1<<5;
    if (result > 0xFFFF) flags_mask |= 1 << 4;

    cpu_registers.f = flags_mask;

    return (uint16_t)result;
}

void comp_r8(uint8_t val){
    alu_sub8(val);
}

//BITWISE LOGIC INSTRUCTIONS
uint8_t and_r8(uint8_t reg){

    uint8_t flags_mask = 0x0;
    uint8_t a_val = cpu_registers.a;

    uint8_t result = a_val & reg;
    
    if (result == 0) flags_mask |= 1 << 7;
    flags_mask |= 1 << 5;

    cpu_registers.f = flags_mask;
    
    return result;
}

uint8_t or_r8(uint8_t reg){

    uint8_t flags_mask = 0x0;
    uint8_t a_val = cpu_registers.a;

    uint8_t result = a_val | reg;

    if (result == 0) flags_mask |= 1 << 7;

    cpu_registers.f = flags_mask;
    return result;
}

uint8_t xor_r8(uint8_t reg){
    
    uint8_t flags_mask = 0x0;
    uint8_t a_val = cpu_registers.a;

    uint8_t result = a_val ^ reg;

    if (result == 0) flags_mask |= 1 << 7;

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t not(){

    uint8_t flags_mask = 0x0;
    uint8_t a_val = cpu_registers.a;

    uint8_t result = ~a_val; //bitwise NOT

    flags_mask |= cpu_registers.f; //keep the value of Z and C flags

    //set N and H flags to 1
    flags_mask |= 1 << 5;
    flags_mask |= 1 << 6;

    cpu_registers.f = flags_mask;

    return result;
}

//BIT SHIFT INSTRUCTIONS
uint8_t rr_r8(uint8_t reg){
    //rotate register reg right through the carry flag

    uint8_t c_flag = (cpu_registers.f >>4) & 1;
    uint8_t lsb = reg & 1;

    uint8_t result = (reg>>1) | (c_flag << 7);

    cpu_registers.f = 0x0;

    if (result == 0) cpu_registers.f |= 1 << 7;
    cpu_registers.f |= (lsb << 4);

    return result;
}

uint8_t rl_r8(uint8_t reg){
    //rotate register reg left through the carry flag

    uint8_t c_flag = (cpu_registers.f >>4) & 1; //get current value of carry flag
    uint8_t msb = reg & 0x80; //get MSB

    uint8_t result = (reg<<1) | c_flag;

    cpu_registers.f = 0x0;

    if (result == 0) cpu_registers.f |= 1 << 7;
    cpu_registers.f |= (msb >> 3); //set new value of C flag to MSB

    return result;
}

uint8_t rrc_r8(uint8_t reg){
    //perform circular right rotation of reg

    //standard right circular byte rotation
    uint8_t lsb = reg & 1;
    uint8_t result = (reg>>1) | (lsb<<7);

    if (result == 0) cpu_registers.f |= 1 << 7;
    cpu_registers.f |= (lsb << 4); //set carry flag to value of LSB

    return result;
}

uint8_t rlc_r8(uint8_t reg){
    //perform circular left rotation of reg

    //standard left circular byte rotation
    uint8_t msb = (reg & 0x80) >> 7;
    uint8_t result = (reg<<1) | msb;

    uint8_t flags_mask = 0x0;

    if (result == 0) flags_mask |= (1 << 7);
    if (msb)         flags_mask |= (1 << 4);

    cpu_registers.f = flags_mask;
    return result;
}