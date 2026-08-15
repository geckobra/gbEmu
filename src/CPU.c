#include "CPU.h"
#include "MMU.h"
#include <string.h>

#define TRACE_SIZE 256

typedef struct {
    uint16_t pc;
    uint8_t opcode;
    uint8_t a, f, b, c, d, e, h, l;
    uint16_t sp;
} CpuTrace;

CpuTrace trace_buffer[TRACE_SIZE];
size_t trace_index = 0;

void record_trace(uint8_t opcode, uint16_t current_pc) {
    trace_buffer[trace_index] = (CpuTrace){
        .pc = current_pc,
        .opcode = opcode,
        .a = cpu_registers.a, .f = cpu_registers.f,
        .b = cpu_registers.b, .c = cpu_registers.c,
        .d = cpu_registers.d, .e = cpu_registers.e,
        .h = cpu_registers.h, .l = cpu_registers.l,
        .sp = cpu_registers.sp
    };
    trace_index = (trace_index + 1) % TRACE_SIZE;
}

void dump_trace(void) {
    printf("\n=== LAST %d EXECUTED INSTRUCTIONS ===\n", TRACE_SIZE);
    for (size_t i = 0; i < TRACE_SIZE; i++) {
        size_t idx = (trace_index + i) % TRACE_SIZE;
        CpuTrace* t = &trace_buffer[idx];
        printf("PC: 0x%04X | OP: 0x%02X | A: %02X F: %02X | BC: %02X%02X | DE: %02X%02X | HL: %02X%02X | SP: %04X\n",
               t->pc, t->opcode, t->a, t->f, t->b, t->c, t->d, t->e, t->h, t->l, t->sp);
    }
}

sm83_registers_t cpu_registers = {};
bool isHalted = false;
bool interrupts_enabled = false;
uint8_t ei_delay = 2;

uint8_t next_instruction = 0x0; //next instruction to be executed
uint64_t total_t_cycles = 0;

FILE* cpu_log_file = NULL;

void init_logging(void) {
    cpu_log_file = fopen("cpu_trace.log", "w");

    if (cpu_log_file == NULL){
        printf("No log file\n");
    }
}

void cpu_init(){
    //initialize registers to default values
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
    if (isHalted) {
        t_cycles_executed = 4;
    } else {
        uint16_t current_pc = cpu_registers.pc;
        next_instruction = fetch();

        if (cpu_log_file) {
            fprintf(cpu_log_file, "PC: 0x%04X | OP: 0x%02X | A: 0x%02X | F: 0x%02X | SP: 0x%04X\n",
                    current_pc, next_instruction, cpu_registers.a, cpu_registers.f, cpu_registers.sp);
        }

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
    uint8_t a_val = cpu_registers.a;
    uint8_t result = a_val - val;
    
    uint8_t flags_mask = (1<<6);
    if (result == 0) flags_mask |= (1 << 7); //set Z flag
    if (((a_val & 0x0F) < (val & 0x0F))) flags_mask |= (1 << 5); //set H-carry flag
    if (val > a_val) flags_mask |= (1 << 4); //if value is higher than regA set Carry flag

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t alu_subc8(uint8_t val){

    uint8_t flags_mask = 1 << 6;

    uint8_t a_val = cpu_registers.a;
    uint8_t carry_flag = (cpu_registers.f & (1 << 4)) ? 1 : 0;

    uint8_t result = a_val - val - carry_flag;

    if (result == 0) flags_mask |= 1 << 7;
    if ((a_val & 0x0F) < ((val & 0x0F) + carry_flag)) flags_mask |= 1 << 5; //set H-carry flag
    if (a_val < (val + carry_flag)) flags_mask |= 1 << 4; //borrow if the total value to be substracted is greater than regA

    cpu_registers.f = flags_mask;

    return result;
}

uint8_t inc_r8(uint8_t reg){
    uint8_t result = reg+1;

    cpu_registers.f &= ~(1 << 7 | 1 << 6 | 1 << 5); // Clear Z, N, H (Keep C!)
    if (result == 0) cpu_registers.f |= (1 << 7);      // Set Z
    if ((reg & 0x0F) == 0x0F) cpu_registers.f |= (1 << 5);
    
    return result;
}

uint8_t dec_r8(uint8_t val) {
    uint8_t res = val - 1;
    
    cpu_registers.f &= ~(1 << 7 | 1 << 5); // Clear Z, H (Keep C!)
    cpu_registers.f |= (1 << 6);           // Set N = 1
    
    if (res == 0) cpu_registers.f |= (1 << 7);      // Set Z
    if ((val & 0x0F) == 0x00) cpu_registers.f |= (1 << 5); // Set H (borrow from bit 4)
    
    return res;
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
    uint8_t a_val = cpu_registers.a;
    uint8_t result = ~a_val; //bitwise NOT

    uint8_t flags_mask = (cpu_registers.f & 0x90) | (1 << 6) | (1 << 5);

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
    uint8_t flags_mask = 0x0;

    //standard right circular byte rotation
    uint8_t lsb = reg & 1;
    uint8_t result = (reg>>1) | (lsb<<7);

    if (result == 0) flags_mask |= 1 << 7;
    flags_mask |= (lsb << 4); //set carry flag to value of LSB

    cpu_registers.f = flags_mask;
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