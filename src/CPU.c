#include "CPU.h"
#include "MMU.h"
#include "timers.h"
#include <stdio.h>
#include <string.h>

bool isHalted = false;
bool interrupts_enabled = false;
bool increment_PC = true;
sm83_registers_t cpu_registers = {};

uint8_t next_instruction = 0x00;
uint64_t total_t_cycles = 0;

uint8_t ei_delay = 2;
static uint8_t IF_byte;
static uint8_t IE_byte;
static uint8_t IR_src; //interrupt to be serviced with the most priority
uint8_t wait_cpu_cycles = 0; //set this value to the number of M-cycles to be waited by the CPU for
static bool waiting_for_interrupt = false;

static bool wait_cpu(){
    //returns true if the CPU is not performing operations in this iteration
    return (wait_cpu_cycles > 0) ? true : false;
}

static void run_interrupts(){
    if (wait_cpu()) return;

    uint8_t handler_addr = 0x00;

    switch (IR_src){
        case 0x01: handler_addr = 0x40; break;
        case 0x02: handler_addr = 0x48; break;
        case 0x04: handler_addr = 0x50; break;
        case 0x08: handler_addr = 0x58; break;
        case 0x10: handler_addr = 0x60; break;
    }

    push(&cpu_registers.sp, cpu_registers.pc);
    cpu_registers.pc = handler_addr;
}

void cpu_init(){
    cpu_registers.a = 0x01;
    cpu_registers.f = 1 << 7;
    cpu_registers.b = 0x00;
    cpu_registers.c = 0x13;
    cpu_registers.d = 0x00;
    cpu_registers.e = 0xD8;
    cpu_registers.h = 0x01;
    cpu_registers.l = 0x4D;
    cpu_registers.sp = 0xFFFE;
    cpu_registers.pc = 0x0100;

    IF_byte = 0x00;
    IE_byte = 0x00;

    timers_init();
}

uint8_t fetch(){
    uint8_t val = read_memory(cpu_registers.pc);

    //the sole purpose of this is to simulate the HALT bug
    if (increment_PC){
        cpu_registers.pc++;
    } else if (!isHalted){
        //PC is incremented again after HALT bug if the last instruction was not a HALT
        increment_PC = true;
    }
    
    return val;
}

void run_cpu(){
    int t_cycles_executed = 0;

    IE_byte = mmu_get_ie();
    IF_byte = mmu_get_if();

    if (ei_delay > 0){
        ei_delay--;
        if (ei_delay == 0){
            interrupts_enabled = true;
            isHalted = false; //when IME is set, the CPU stops being halted
        }
    }

    if (isHalted && (IE_byte & IF_byte)){
        //CPU stops HALT when an interrupt is pending
        isHalted = false;
        
        if (!interrupts_enabled) increment_PC = false; //HALT bug -> if HALT is executed, IME = 0 and IE & IF != 0, PC is not incremented
    }
    
    //printf("IE_byte %2x | IF_byte: %2x \n", IE_byte, IF_byte);
    if (wait_cpu()){
        wait_cpu_cycles--;
        t_cycles_executed = 4;

        if (wait_cpu_cycles == 0 && waiting_for_interrupt){
            waiting_for_interrupt = false;
            run_interrupts();
        }
    } else if (isHalted){
        t_cycles_executed = 4;
    } else{

        //if interrupts are enabled and some interrupt with a handler has ocurred, service that interruption
        if (interrupts_enabled && (IE_byte & IF_byte & 0x1F)){
            interrupts_enabled = false;
            waiting_for_interrupt = true;
            wait_cpu_cycles = 2;

            for (size_t i = 0; i < 5; i++){
                //check the bytes and exit the loop at the first interrupt source (which will be the one with the most priority)
                if ((IE_byte & IF_byte) & (1 << i)){
                    IF_byte &= ~(1 << i);
                    IR_src = (1 << i);
                    break;
                }
            }

            //update the IF_byte in memory
            write_memory(0xFF0F, IF_byte);
            t_cycles_executed = 4;
            wait_cpu_cycles--;
        } else {
            next_instruction = fetch();
            //printf("Running %2X\n", next_instruction);
            t_cycles_executed = execute(next_instruction);
        }
    }

    total_t_cycles += t_cycles_executed;

    tick_timers(t_cycles_executed);
}

// ALU HELPER FUNCTIONS
uint8_t alu_add8(uint8_t val){
    uint8_t flags_mask = 0x00;
    uint8_t a_val = cpu_registers.a;

    uint16_t result = (uint16_t)a_val + (uint16_t)val;

    if ((uint8_t)result == 0)                  flags_mask |= 1 << 7;
    if ((a_val & 0x0F) + (val & 0x0F) > 0x0F)  flags_mask |= 1 << 5;
    if (result > 0xFF)                         flags_mask |= 1 << 4;
    
    cpu_registers.f = flags_mask;

    return (uint8_t)result;
}

uint8_t alu_adc8(uint8_t val){
    uint8_t flags_mask = 0x00;
    uint8_t a_val = cpu_registers.a;

    uint8_t carry_flag = (cpu_registers.f & (1<<4)) ? 1 : 0;

    uint16_t result = (uint16_t)a_val + (uint16_t)val + carry_flag;

    if ((uint8_t)result == 0)                               flags_mask |= 1 << 7;
    if ((a_val & 0x0F) + (val & 0x0F) + carry_flag > 0x0F)  flags_mask |= 1 << 5;
    if (result > 0xFF)                                      flags_mask |= 1 << 4;

    cpu_registers.f = flags_mask;
    return (uint8_t)result;
}

uint8_t alu_sub8(uint8_t val){
    uint8_t a_val = cpu_registers.a;
    uint8_t result = a_val - val;
    
    uint8_t flags_mask = (1<<6);
    if (result == 0) flags_mask |= (1 << 7);
    if ((a_val & 0x0F) < (val & 0x0F)) flags_mask |= (1 << 5);
    if (val > a_val) flags_mask |= (1 << 4);

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t alu_subc8(uint8_t val){
    uint8_t flags_mask = 1 << 6;

    uint8_t a_val = cpu_registers.a;
    uint8_t carry_flag = (cpu_registers.f & (1 << 4)) ? 1 : 0;

    uint8_t result = a_val - val - carry_flag;

    if (result == 0) flags_mask |= 1 << 7;
    if ((a_val & 0x0F) < ((val & 0x0F) + carry_flag)) flags_mask |= 1 << 5;
    if (a_val < (val + carry_flag)) flags_mask |= 1 << 4;

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t inc_r8(uint8_t reg){
    uint8_t result = reg+1;

    cpu_registers.f &= ~(1 << 7 | 1 << 6 | 1 << 5);
    if (result == 0) cpu_registers.f |= (1 << 7);
    if ((reg & 0x0F) == 0x0F) cpu_registers.f |= (1 << 5);
    
    return result;
}

uint8_t dec_r8(uint8_t val) {
    uint8_t res = val - 1;
    
    cpu_registers.f &= ~(1 << 7 | 1 << 5);
    cpu_registers.f |= (1 << 6);
    
    if (res == 0) cpu_registers.f |= (1 << 7);
    if ((val & 0x0F) == 0x00) cpu_registers.f |= (1 << 5);
    
    return res;
}

uint16_t alu_add16(uint16_t reg1, uint16_t reg2){
    uint8_t flags_mask = 0x0;
    uint32_t result = reg1+reg2;

    flags_mask |= (cpu_registers.f & 0x80);

    if ((reg1 & 0x0FFF) + (reg2 & 0x0fFF) > 0x0FFF) flags_mask |= 1<<5;
    if (result > 0xFFFF) flags_mask |= 1 << 4;

    cpu_registers.f = flags_mask;

    return (uint16_t)result;
}

void comp_r8(uint8_t val){
    alu_sub8(val);
}

// BITWISE LOGIC INSTRUCTIONS
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
    uint8_t a_val = cpu_registers.a;
    uint8_t result = ~a_val;

    uint8_t flags_mask = (cpu_registers.f & 0x90) | (1 << 6) | (1 << 5);

    cpu_registers.f = flags_mask;
    return result;
}

// BIT SHIFT INSTRUCTIONS
uint8_t rr_r8(uint8_t reg){
    uint8_t c_flag = (cpu_registers.f >>4) & 1;
    uint8_t lsb = reg & 1;

    uint8_t result = (reg>>1) | (c_flag << 7);

    cpu_registers.f = 0x0;

    if (result == 0) cpu_registers.f |= 1 << 7;
    cpu_registers.f |= (lsb << 4);

    return result;
}

uint8_t rl_r8(uint8_t reg){
    uint8_t c_flag = (cpu_registers.f >>4) & 1;
    uint8_t msb = reg & 0x80;

    uint8_t result = (reg<<1) | c_flag;

    cpu_registers.f = 0x0;

    if (result == 0) cpu_registers.f |= 1 << 7;
    cpu_registers.f |= (msb >> 3);

    return result;
}

uint8_t rrc_r8(uint8_t reg){
    uint8_t flags_mask = 0x0;

    uint8_t lsb = reg & 1;
    uint8_t result = (reg>>1) | (lsb<<7);

    if (result == 0) flags_mask |= 1 << 7;
    flags_mask |= (lsb << 4);

    cpu_registers.f = flags_mask;
    return result;
}

uint8_t rlc_r8(uint8_t reg){
    uint8_t msb = (reg & 0x80) >> 7;
    uint8_t result = (reg<<1) | msb;

    uint8_t flags_mask = 0x0;

    if (result == 0) flags_mask |= (1 << 7);
    if (msb)         flags_mask |= (1 << 4);

    cpu_registers.f = flags_mask;
    return result;
}

uint8_t sla_r8(uint8_t reg){
    uint8_t msb        = (reg & 0x80)>>7; //get most significant bit
    uint8_t result     = reg << 1;
    uint8_t flags_mask = 0x00;

    if (result == 0) flags_mask |= (1<<7);
    flags_mask |= (msb << 4); //set carry flag if accordingly

    cpu_registers.f = flags_mask;
    return result;
}

uint8_t sra_r8(uint8_t reg){
    uint8_t lsb = (reg & 0x01); //get least significant bit to set carry flag
    uint8_t msb = (reg & 0x80);
    uint8_t result = (reg >> 1) | msb;
    uint8_t flags_mask = 0x00;

    if (result == 0) flags_mask |= (1<<7);
    flags_mask |= (lsb << 4);

    cpu_registers.f = flags_mask;
    return result;
}

uint8_t srl_r8(uint8_t reg){
    uint8_t flags_mask = 0x00;
    uint8_t result = 0x00;
    uint8_t lsb = reg & 0x01;
    
    result |= reg >> 1;

    if (result == 0) flags_mask |= (1<<7);
    flags_mask |= (lsb << 4);

    cpu_registers.f = flags_mask;
    return result;
}

uint8_t swap(uint8_t reg){
    uint8_t reg_lo = reg & 0x0F;
    uint8_t reg_hi = reg & 0xF0;

    uint8_t res = (reg_lo << 4) | (reg_hi >> 4);

    cpu_registers.f = 0x00; //reset al flags
    if (res == 0) cpu_registers.f = (1 << 7); //set Z if result is zero

    return res;
}

void bit_r8(uint8_t bit_pos, uint8_t reg){
    uint8_t flags_mask = (cpu_registers.f & (1 << 4)); //preserve carry flag
    uint8_t bit_val = ((reg >> bit_pos) & 1);          //shift the register by the bit_pos value and mask it

    if (bit_val == 0) flags_mask |= (1 << 7);          //then set the zero flag if the bit is zero
    flags_mask |= (1 << 5);

    cpu_registers.f = flags_mask;
}

uint8_t res_r8(uint8_t bit_pos, uint8_t reg){
    uint8_t mask = ~(1<<bit_pos); //create a mask with all 1s except the bit position that needs to be zero
    uint8_t result = reg & mask;  //keeps old values and sets that specific bit to zero if it was 1

    return result;
}

uint8_t set_r8(uint8_t bit_pos, uint8_t reg){
    uint8_t mask = (1<<bit_pos); //create a mask with all 0s except the bit position that needs to be one
    uint8_t result = reg | mask;  //keeps old values and sets that specific bit to one if it was zero

    return result;
}