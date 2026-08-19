#include "CPU.h"
#include "MMU.h"
#include "timers.h"

uint8_t get_cb_operand(uint8_t reg_bits) {
    switch (reg_bits & 0x07) {
        case 0: return cpu_registers.b;
        case 1: return cpu_registers.c;
        case 2: return cpu_registers.d;
        case 3: return cpu_registers.e;
        case 4: return cpu_registers.h;
        case 5: return cpu_registers.l;
        case 6: tick_timers(4); return read_memory((cpu_registers.h << 8) | cpu_registers.l); // [HL]
        case 7: return cpu_registers.a;
    }
    return 0;
}

void write_cb_operand(uint8_t reg_bits, uint8_t val) {
    switch (reg_bits & 0x07) {
        case 0: cpu_registers.b = val; break;
        case 1: cpu_registers.c = val; break;
        case 2: cpu_registers.d = val; break;
        case 3: cpu_registers.e = val; break;
        case 4: cpu_registers.h = val; break;
        case 5: cpu_registers.l = val; break;
        case 6: tick_timers(4); write_memory((cpu_registers.h << 8) | cpu_registers.l, val); break; // [HL]
        case 7: cpu_registers.a = val; break;
    }
}

int dispatch_cb(uint8_t instruction){
    int executed_t_ticks = 0;
    uint8_t reg_bits = instruction & 0x07;
    uint8_t operand = get_cb_operand(reg_bits);
    uint8_t result = 0;

    uint8_t operation_type = (instruction >> 3) & 0x1F;

    if (operation_type >= 0x08 && operation_type <= 0x0F) {
        uint8_t bit_pos = (instruction >> 3) & 0x07;
        bit_r8(bit_pos, operand);
        // return (reg_bits == 6) ? 12 : 8;
        return 8;
    }

    if (operation_type >= 0x10 && operation_type <= 0x17) {
        uint8_t bit_pos = (instruction >> 3) & 0x07;
        result = res_r8(bit_pos, operand);
        write_cb_operand(reg_bits, result);
        return 8;
    }

    if (operation_type >= 0x18 && operation_type <= 0x1F) {
        uint8_t bit_pos = (instruction >> 3) & 0x07;
        result = set_r8(bit_pos, operand);
        write_cb_operand(reg_bits, result);
        return 8;
    }

    switch (operation_type){
        case 0x00: result = rlc_r8(operand); break;
        case 0x01: result = rrc_r8(operand); break;
        case 0x02: result = rl_r8(operand);  break;
        case 0x03: result = rr_r8(operand);  break;
        case 0x04: result = sla_r8(operand); break;
        case 0x05: result = sra_r8(operand); break;
        case 0x06: result = swap(operand);   break;
        case 0x07: result = srl_r8(operand); break;
    }

    write_cb_operand(reg_bits, result);
    executed_t_ticks = 8;

    return executed_t_ticks;
}