#include "CPU.h"
#include "MMU.h"

uint8_t get_operand(uint8_t reg_bits){
    switch (reg_bits & 0x07){
        case 0: return cpu_registers.b;
        case 1: return cpu_registers.c;
        case 2: return cpu_registers.d;
        case 3: return cpu_registers.e;
        case 4: return cpu_registers.h;
        case 5: return cpu_registers.l;
        case 6: return read_memory(cpu_registers.hl);
        case 7: return cpu_registers.a;
        default: return 0x00;
    }
}

void write_result(uint8_t res, uint8_t dest){
    switch (dest){
        case 0x00: cpu_registers.b = res; break;
        case 0x01: cpu_registers.c = res; break;
        case 0x02: cpu_registers.d = res; break;
        case 0x03: cpu_registers.e = res; break;
        case 0x04: cpu_registers.h = res; break;
        case 0x05: cpu_registers.l = res; break;
        case 0x06: write_memory(cpu_registers.hl, res); break;
        case 0x07: cpu_registers.a = res; break;
    }
}

//returns the T-Cycles needed to run the instruction and advances the PC as needed
int execute(uint8_t opcode){

    int executed_t_ticks = 0;
    uint8_t value = 0; //8 bit value to extract from the instruction
    uint8_t reg_bits = opcode & 0x07;

    if (opcode == 0x76){
        //HALT isntruction is in the middle of the LD quadrant
        isHalted = true;
        executed_t_ticks = 4;
        return executed_t_ticks;
    } else if ((opcode & 0xC0) == 0x40){
        //LOAD instructions write directly to destiny register
        uint8_t operand = get_operand(reg_bits);
        uint8_t dest = (opcode >> 3)& 0x07; //get destiny register
        write_result(operand, dest);

        executed_t_ticks = ((reg_bits == 6) || (dest == 6)) ? 8 : 4;
        return executed_t_ticks;
    }

    //if the operation belongs to the ALU quadrant
    if ((opcode & 0xC0) == 0x80){
        uint8_t operand = get_operand(reg_bits);
        uint8_t alu_op = (opcode >> 3) & 0x07; //ALU operation is encoded in the bits 5, 4 and 3
        switch (alu_op){
            case 0x0:
                cpu_registers.a = alu_add8(operand);
                executed_t_ticks = ((reg_bits == 6)) ? 8 : 4;
                break;

            case 0x01:
                cpu_registers.a = alu_adc8(operand);
                executed_t_ticks = ((reg_bits == 6)) ? 8 : 4;
                break;

            case 0x02:
                cpu_registers.a = alu_sub8(operand);
                executed_t_ticks = (reg_bits == 6) ? 8 : 4;
                break;

            case 0x03:
                cpu_registers.a = alu_subc8(operand);
                executed_t_ticks = (reg_bits == 6) ? 8 : 4;
                break;

            case 0x04:
                cpu_registers.a = and_r8(operand);
                executed_t_ticks = (reg_bits == 6) ? 8 : 4;
                break;

            case 0x05:
                cpu_registers.a = xor_r8(operand);
                executed_t_ticks = (reg_bits == 6) ? 8 : 4;
                break;

            case 0x06:
                cpu_registers.a = or_r8(operand);
                executed_t_ticks = (reg_bits == 6) ? 8 : 4;
                break;

            case 0x07:
                comp_r8(operand);
                executed_t_ticks = (reg_bits == 6) ? 8 : 4;
                break;
        }

        return executed_t_ticks;
    }

    switch(opcode){
        case 0x0:
            //NOP, doesn't do anything
            executed_t_ticks = 4;
            break;

        case 0x01:{
            //load n16 value into BC register

            //get the value from the instruction. Little-Endian so Low byte is first
            uint8_t val_l = fetch();
            uint8_t val_h = fetch();

            cpu_registers.bc = ((uint16_t)val_h<<8) | val_l; //load the value into the bc register

            executed_t_ticks = 12;

            break;
        }

        case 0x02:
            //load the value stored in A at the memory pointed to by BC
            write_memory(cpu_registers.bc, cpu_registers.a);

            executed_t_ticks = 8;

            break;

        case 0x03:
            //increment BC register
            cpu_registers.bc++;
            executed_t_ticks = 8;
            break;

        case 0x04:
            //increment B register setting corresponding flags
            cpu_registers.b = inc_r8(cpu_registers.b);
            executed_t_ticks = 4;
            break;

        case 0x05:
            //decrement regB and set flags accordingly
            cpu_registers.b = dec_r8(cpu_registers.b);
            executed_t_ticks = 4;
            break;

        case 0x06:
            //load n8 value into regB
            value = fetch();

            cpu_registers.b = value;
            
            executed_t_ticks = 8;
            break;

        case 0x07:
            //perform left circular rotation on regA
            cpu_registers.a = rlc_r8(cpu_registers.a);

            cpu_registers.f &= ~(1 << 7); //this instruction resets zero flag (bit 7)

            executed_t_ticks = 4;
            break;

        case 0x08: {
            //load the stack pointer at WRAM[n16]
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t address = ((uint16_t)h_val<<8) | l_val;

            uint8_t sp_h = (cpu_registers.sp >> 8) & 0xFF;
            uint8_t sp_l = (cpu_registers.sp) & 0x00FF;

            write_memory(address, sp_l);
            write_memory(address+1, sp_h);
            executed_t_ticks = 20;
            break;
        }

        case 0x09:
            //add contents of regBC to regHL
            cpu_registers.hl = alu_add16(cpu_registers.hl, cpu_registers.bc);

            executed_t_ticks = 8;
            break;

        case 0x0A:
            //load in regA the value stored at WRAM[BC]
            cpu_registers.a = read_memory(cpu_registers.bc);
            executed_t_ticks = 8;
            break;

        case 0x0B:
            //decrement regBC by one
            cpu_registers.bc--;

            executed_t_ticks = 8;
            break;

        case 0x0C:
            //increment regC and set corresponding flags
            cpu_registers.c = inc_r8(cpu_registers.c);
            executed_t_ticks = 4;
            break;

        case 0x0D:
            //decrement regC and set flags accordingly
            cpu_registers.c = dec_r8(cpu_registers.c);

            executed_t_ticks = 4;
            break;

        case 0x0E:
            //load an 8 bit value in regC
            value = fetch();
            cpu_registers.c = value;
            executed_t_ticks = 8;
            break;

        case 0x0F:
            //perform circular right rotation on regA
            cpu_registers.a = rrc_r8(cpu_registers.a);

            cpu_registers.f &= ~(1 << 7); //this instruction resets zero flag (bit 7)

            executed_t_ticks = 4;
            break;

        case 0x10:
            //STOP instruction -> enters VERY low power mode. Stops CPU and LCD until a button press occurs.
            value = fetch(); //STOP is two bytes, the second always being 0x00 and unused.

            //halt the CPU
            isHalted = true;

            executed_t_ticks = 4;

            //currently the LCD controller is not implemented
            break;

        case 0x11:{
            //load n16 value into regDE
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t val = ((uint16_t)h_val<<8) | l_val;
            cpu_registers.de = val;
            executed_t_ticks = 12;
            break;
        }

        case 0x12:
            //load in WRAM[DE] the value stored at regA
            write_memory(cpu_registers.de, cpu_registers.a);
            executed_t_ticks = 8;
            break;

        case 0x13:
            //increment regDE
            cpu_registers.de++;
            executed_t_ticks = 8;
            break;

        case 0x14:
            //increment regD setting corresponding flags
            cpu_registers.d = inc_r8(cpu_registers.d);
            executed_t_ticks = 4;
            break;

        case 0x15:
            //decrement regD and set flags accordingly
            cpu_registers.d = dec_r8(cpu_registers.d);
            executed_t_ticks = 4;
            break;

        case 0x16:
            //load an 8 bit value in regD
            value = fetch();
            cpu_registers.d = value;
            executed_t_ticks = 8;
            break;

        case 0x17:
            //perform left rotation through carry flag on regA
            cpu_registers.a = rl_r8(cpu_registers.a);

            cpu_registers.f &= ~(1 << 7);

            executed_t_ticks = 4;
            break;

        case 0x18:
            //relative jump to pc+offset @ n8
            value = fetch();
            cpu_registers.pc += (int8_t)value;

            executed_t_ticks = 8;

            break;

        case 0x19:
            //add the contents of regHL and regDE
            cpu_registers.hl = alu_add16(cpu_registers.hl, cpu_registers.de);

            executed_t_ticks = 8;
            break;
        
        case 0x1A:
            //load value at WRAM[DE] into regA
            cpu_registers.a = read_memory(cpu_registers.de);
            executed_t_ticks = 8;
            break;

        case 0x1B:
            //decrement regDE by one
            cpu_registers.de--;

            executed_t_ticks = 8;
            break;

        case 0x1C:
            //increment regE and set corresponding flags
            cpu_registers.e = inc_r8(cpu_registers.e);
            executed_t_ticks = 4;
            break;

        case 0x1D:
            //decrement regE and set flags accordingly
            cpu_registers.e = dec_r8(cpu_registers.e);

            executed_t_ticks = 4;
            break;

        case 0x1E:
            //load 8 bit value into regE
            value = fetch();
            cpu_registers.e = value;
            executed_t_ticks = 8;
            break;

        case 0x1F:
            //perform right rotation through carry flag on regA
            cpu_registers.a = rr_r8(cpu_registers.a);

            cpu_registers.f &= ~(1 << 7);
            executed_t_ticks = 4;
            break;

        case 0x20:{
            value = fetch();
            uint8_t zero_flag = (cpu_registers.f >> 7) & 1;

            if (zero_flag){
                executed_t_ticks = 8;
            } else{
                cpu_registers.pc += (int8_t)value;
                executed_t_ticks = 12;
            }
            
            break;
        }

        case 0x21:{
            //load n16 value into regHL
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t val = ((uint16_t)h_val<<8) | l_val;
            cpu_registers.hl = val;
            executed_t_ticks = 12;
            break;
        }

        case 0x22:
            //load regA into WRAM[HL] and increment HL by 1
            write_memory(cpu_registers.hl, cpu_registers.a);
            cpu_registers.hl++;
            executed_t_ticks = 8;
            break;

        case 0x23:
            //increment HL register
            cpu_registers.hl++;
            executed_t_ticks = 8;
            break;

        case 0x24:
            //increment regH setting corresponding flags
            cpu_registers.h = inc_r8(cpu_registers.h);
            executed_t_ticks = 4;
            break;

        case 0x25:
            //decrement regH by one and set flags accordingly
            cpu_registers.h = dec_r8(cpu_registers.h);

            executed_t_ticks = 4;
            break;

        case 0x26:
            //load n8 value into regH
            value = fetch();
            cpu_registers.h = value;
            executed_t_ticks = 8;
            break;

        case 0x27:{
            //DAA instruction, to be used after performing arithmetic instruction
            uint8_t adjustment = 0;

            uint8_t hc_flag = (cpu_registers.f >> 5) & 1;
            uint8_t c_flag = (cpu_registers.f >> 4) & 1;
            uint8_t n_flag = (cpu_registers.f >> 6) & 1;
            
            if (hc_flag || (!n_flag && (cpu_registers.a & 0x0F) > 0x09)) adjustment |= 0x06;

            if (c_flag || (!n_flag && (cpu_registers.a > 0x99))){
                adjustment |= 0x60;
                c_flag = true;
            }

            cpu_registers.a += n_flag ? -adjustment : adjustment;

            uint8_t flags_mask = cpu_registers.f & (1 << 6); //N flag is preserved
            if (cpu_registers.a == 0) flags_mask |= 1 << 7;
            if (c_flag) flags_mask |= 1 << 4;

            flags_mask &= 0xF0;

            cpu_registers.f = flags_mask;
            executed_t_ticks = 4;
            break;
        }

        case 0x28:{
            value = fetch();
            uint8_t zero_flag = (cpu_registers.f >> 7) & 1;

            if (zero_flag){
                cpu_registers.pc += (int8_t)value;
                executed_t_ticks = 12;
            } else{
                executed_t_ticks = 8;
            }

            break;
        }

        case 0x29:
            //add the contents of regHL and regHL
            cpu_registers.hl = alu_add16(cpu_registers.hl, cpu_registers.hl);

            executed_t_ticks = 8;
            break;

        case 0x2A:
            //load WRAM[regHL] into regA and increment regHL by 1
            cpu_registers.a = read_memory(cpu_registers.hl);
            cpu_registers.hl++;
            executed_t_ticks = 8;
            break;

        case 0x2B:
            //decrement register HL by one
            cpu_registers.hl--;

            executed_t_ticks = 8;
            break;

        case 0x2C:
            //increment regL and set corresponding flags
            cpu_registers.l = inc_r8(cpu_registers.l);
            executed_t_ticks = 4;
            break;

        case 0x2D:
            //decrement regL and set flags accordingly
            cpu_registers.l = dec_r8(cpu_registers.l);

            executed_t_ticks = 4;
            break;

        case 0x2E:
            //load n8 value into regL
            value = fetch();
            cpu_registers.l = value;
            executed_t_ticks = 8;
            break;

        case 0x2F:
            cpu_registers.a = ~cpu_registers.a;
            cpu_registers.f |= (1 << 6) | (1 << 5); // N = 1, H = 1
            executed_t_ticks = 4;
            break;

        case 0x30:{
            //jump to PC+e8 if flag C is not set
            value = fetch();
            uint8_t carry_flag = (cpu_registers.f >> 4) & 1;

            if (carry_flag){
                executed_t_ticks = 8;
            } else {
                cpu_registers.pc += (int8_t)value;
                executed_t_ticks = 12;
            }

            break;
        }

        case 0x31:{
            //load n16 value into SP
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t val = ((uint16_t)h_val<<8) | l_val;
            cpu_registers.sp = val;
            executed_t_ticks = 12;
            break;
        }

        case 0x32:
            //load regA into WRAM[regHL] and decrement HL by 1
            write_memory(cpu_registers.hl, cpu_registers.a);
            cpu_registers.hl--;
            executed_t_ticks = 8;
            break;

        case 0x33:
            //increment regSP
            cpu_registers.sp++;
            executed_t_ticks = 8;
            break;

        case 0x34:{
            //increment WRAM[regHL] and set corresponding flags
            uint8_t value = read_memory(cpu_registers.hl);
            value = inc_r8(value);
            write_memory(cpu_registers.hl, value);

            executed_t_ticks = 12;
            break;
        }

        case 0x35:
            //decrement WRAM[regHL] and set flags accordingly
            value = read_memory(cpu_registers.hl);
            value = dec_r8(value);
            write_memory(cpu_registers.hl, value);

            executed_t_ticks = 12;
            break;

        case 0x36:
            //write n8 value into WRAM[regHL]
            value = fetch();
            write_memory(cpu_registers.hl, value);
            executed_t_ticks  = 12;
            break;

        case 0x37:{
            //set carry flag
            uint8_t flags_mask = 0x0; //clear all flags

            flags_mask |= cpu_registers.f & (1<<7); //preserve zero flag
            flags_mask |= 1 << 4;     //set carry flag afterwards

            cpu_registers.f = flags_mask;
            executed_t_ticks = 4;
            break;
        }

        case 0x38:{
            value = fetch();
            uint8_t carry_flag = (cpu_registers.f >> 4) & 1;

            if (carry_flag){
                cpu_registers.pc += (int8_t)value;
                executed_t_ticks = 12;
            } else{
                executed_t_ticks = 8;
            }

            break;
        }

        case 0x39:
            //add the contents of regHL and regSP

            cpu_registers.hl = alu_add16(cpu_registers.hl, cpu_registers.sp);

            executed_t_ticks = 8;
            break;

        case 0x3A:
            //load value at WRAM[regHL] into regA and decrement HL by 1
            cpu_registers.a = read_memory(cpu_registers.hl);
            cpu_registers.hl--;
            executed_t_ticks = 8;
            break;

        case 0x3B:
            //decrement SP by one
            cpu_registers.sp--;

            executed_t_ticks = 8;
            break;

        case 0x3D:
            //decrement regA and set flags accordingly
            cpu_registers.a = dec_r8(cpu_registers.a);

            executed_t_ticks = 4;
            break;

        case 0x3C:
            //increment regA and set corresponding flags
            cpu_registers.a = inc_r8(cpu_registers.a);
            executed_t_ticks = 4;
            break;

        case 0x3E:
            //load n8 value into regA
            value = fetch();
            cpu_registers.a = value;
            executed_t_ticks = 8;
            break;

        case 0x3F: {
            uint8_t old_c = (cpu_registers.f >> 4) & 1; //get value of old carry flag
            uint8_t z_flag = cpu_registers.f & (1 << 7);
            uint8_t new_c = (old_c) ? 0 : 1;

            cpu_registers.f = z_flag | (new_c << 4);
            executed_t_ticks = 4;
            break;
        }

        case 0xC0:
            //return from subroutine if Z flag is not set

            if ((cpu_registers.f & 0x80)>>7){
                //if Z is set, don't return from subroutine and take 8 T-Cycles
                executed_t_ticks = 8;
            } else{
                cpu_registers.pc = pop(&cpu_registers.sp);
                executed_t_ticks = 20;
            }

            break;
            
        case 0xC1:
            //POP 16 bits from the stack and store at BC
            cpu_registers.bc = pop(&cpu_registers.sp);

            executed_t_ticks = 12;
            break;

        case 0xC2:{
            //absolute jump to address n16 if flag zero is not set
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();

            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            uint8_t zero_flag = (cpu_registers.f >> 7) & 1;

            if (zero_flag){
                executed_t_ticks = 12;
            } else {
                cpu_registers.pc = address;
                executed_t_ticks = 16;
            }

            break;
        }

        case 0xC3:{
            //absolute jump to n16 address
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();

            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            cpu_registers.pc = address;
            executed_t_ticks = 16;

            break;
        }

        case 0xC4:{
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();
            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            uint8_t zero_flag = (cpu_registers.f >> 7) & 1;

            if (zero_flag){
                executed_t_ticks = 12;
            } else{
                push(&cpu_registers.sp, cpu_registers.pc);

                cpu_registers.pc = address;
                executed_t_ticks = 24;
            }
            break;
        }

        case 0xC5:
            //PUSH regBC into the stack
            push(&cpu_registers.sp, cpu_registers.bc);

            executed_t_ticks = 16;
            break;

        case 0xC6:
            //add n8 to regA
            value = fetch();
            cpu_registers.a = alu_add8(value);

            executed_t_ticks = 8;
            break;

        case 0xC7:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x0000;
            executed_t_ticks = 16;
            break;

        case 0xC8:
            //return from subroutine if Z is set

            if ((cpu_registers.f & 0x80)>>7){
                cpu_registers.pc = pop(&cpu_registers.sp);
                executed_t_ticks = 20;
            } else{
                executed_t_ticks = 8;
            }

            break;

        case 0xC9:
            //return from subroutine, this is basically POP PC
            cpu_registers.pc = pop(&cpu_registers.sp);

            executed_t_ticks = 16;
            break;

        case 0xCA:{
            //jump to address n16 if zero flag is set
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();

            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;
            uint8_t zero_flag = (cpu_registers.f >> 7) & 1;

            if (zero_flag){
                cpu_registers.pc = address;
                executed_t_ticks = 16;
            } else{
                executed_t_ticks = 12;
            }

            break;
        }

        case 0xCB:
            value = fetch();
            executed_t_ticks = 4 + dispatch_cb(value); //4 ticks + ticks for CB operation
            break;

        case 0xCC:{
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();
            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            uint8_t zero_flag = (cpu_registers.f >> 7) & 1;

            if (zero_flag){
                push(&cpu_registers.sp, cpu_registers.pc);
                cpu_registers.pc = address;
                executed_t_ticks = 24;
            } else{
                executed_t_ticks = 12;
            }

            break;
        }

        case 0xCD:{
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();
            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = address;
            executed_t_ticks = 24;

            break;
        }

        case 0xCE:
            //add n8 to regA with carry
            value = fetch();
            cpu_registers.a = alu_adc8(value);
            
            executed_t_ticks = 8;
            break;

        case 0xCF:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x0008;
            executed_t_ticks = 16;
            break;

        case 0xD0:
            //return from subroutine if C flag is not set

            if ((cpu_registers.f & 0x10) >> 4){
                executed_t_ticks = 8;
            } else{
                cpu_registers.pc = pop(&cpu_registers.sp);
                executed_t_ticks = 20;
            }

            break;

        case 0xD1:
            cpu_registers.de = pop(&cpu_registers.sp);
            executed_t_ticks = 12;
            break;

        case 0xD2:{
            //jump to address n16 if carry flag is not set
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();

            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;
            uint8_t carry_flag = (cpu_registers.f >> 4) & 1;

            if (carry_flag){
                executed_t_ticks = 12;
            } else{
                cpu_registers.pc = address;
                executed_t_ticks = 16;
            }

            break;
        }

        case 0xD4:{
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();
            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            uint8_t carry_flag = (cpu_registers.f >> 4) & 1;

            if (carry_flag){
                executed_t_ticks = 12;
            } else{
                push(&cpu_registers.sp, cpu_registers.pc);

                cpu_registers.pc = address;
                executed_t_ticks = 24;
            }

            break;
        }

        case 0xD5:
            push(&cpu_registers.sp, cpu_registers.de);
            executed_t_ticks = 16;
            break;

        case 0xD6:
            //substract n8 from regA
            value = fetch();
            cpu_registers.a = alu_sub8(value);

            executed_t_ticks = 8;
            break;

        case 0xD7:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x10;
            executed_t_ticks = 16;
            break;

        case 0xD8:
            //return from subroutine if flag C is set

            if ((cpu_registers.f & 0x10)>>4){
                cpu_registers.pc = pop(&cpu_registers.sp);
                executed_t_ticks = 20;
            } else{
                executed_t_ticks = 8;
            }

            break;
        
        case 0xD9:
            //return from subroutine and enable interrupts 

            cpu_registers.pc = pop(&cpu_registers.sp);
            executed_t_ticks = 16;

            interrupts_enabled = true;
            break;

        case 0xDA:{
            //jump to address n16 if carry flag is set
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();

            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;
            uint8_t carry_flag = (cpu_registers.f >> 4) & 1;

            if (carry_flag){
                cpu_registers.pc = address;
                executed_t_ticks = 16;
            } else{
                executed_t_ticks = 12;
            }

            break;
        }

        case 0xDC:{
            uint8_t addr_low = fetch();
            uint8_t addr_high = fetch();
            uint16_t address = ((uint16_t)addr_high << 8) | addr_low;

            uint8_t carry_flag = (cpu_registers.f >> 4) & 1;

            if (carry_flag){
                push(&cpu_registers.sp, cpu_registers.pc);

                cpu_registers.pc = address;
                executed_t_ticks = 24;
            } else{
                executed_t_ticks = 12;
            }
            break;
        }

        case 0xDE:  
            //substract n8 from regA with carry
            value = fetch();
            cpu_registers.a = alu_subc8(value);

            executed_t_ticks = 8;
            break;

        case 0xDF:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x18;
            executed_t_ticks = 16;
            break;

        case 0xE0: {
            //write the contents of regA to HIGH WRAM[n8]
            value = fetch(); //get the address

            //the address is encoded as the 8-bit low byte and assumes high byte of $FF
            uint16_t address = 0xFF00;
            address = address | (uint16_t)value;

            write_memory(address, cpu_registers.a);
            executed_t_ticks = 12;
            break;
        }

        case 0xE1:
            //POP 16 bits from stack and store them at HL
            cpu_registers.l = read_memory(cpu_registers.sp++);
            cpu_registers.h = read_memory(cpu_registers.sp++);

            executed_t_ticks = 12;
            break;

        case 0xE2: {
            //write the contents of regA to HIGH WRAM[regC]
            uint16_t address = 0xFF00;
            address = address | (uint16_t)cpu_registers.c;

            write_memory(address, cpu_registers.a);
            executed_t_ticks = 8;
            break;
        }

        case 0xE5:
            //PUSH regHL into the stack
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.h);
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.l);

            executed_t_ticks = 16;
            break;

        case 0xE6:
            //set contents of regA to regA & n8
            value = fetch();
            cpu_registers.a = and_r8(value);

            executed_t_ticks = 8;
            break;

        case 0xE7:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x20;
            executed_t_ticks = 16;
            break;

        case 0xE8:{
            //add e8 value to the stack pointer
            int8_t val = (int8_t)fetch();
            uint8_t unsigned_val = (uint8_t)val;
            uint8_t sp_low = (uint8_t)(cpu_registers.sp & 0xFF);

            cpu_registers.f = 0x0;

            if ((sp_low & 0x0F )+ (unsigned_val & 0x0F) > 0x0F) cpu_registers.f |= 1 << 5;
            if (sp_low + unsigned_val > 0xFF) cpu_registers.f |= 1 << 4;

            cpu_registers.sp = (uint16_t)((int32_t)cpu_registers.sp + val);
            executed_t_ticks = 16;
            break;
        }

        case 0xE9:{
            //jump to address HL
            cpu_registers.pc = cpu_registers.hl;

            executed_t_ticks = 4;
            break;
        }

        case 0xEA:{
            //write the contents of regA to WRAM[n16]
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t res_address = ((uint16_t)h_val << 8) | l_val;
            write_memory(res_address, cpu_registers.a);
            executed_t_ticks = 16;
            break;
        }

        case 0xEE:
            //set the contents of regA to regA ^ n8
            value = fetch();
            cpu_registers.a = xor_r8(value);

            executed_t_ticks = 8;
            break;

        case 0xEF:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x28;
            executed_t_ticks = 16;
            break;

        case 0xF0: {
            //load the contents at HIGH WRAM[n8] into regA
            value = fetch();
            uint16_t address = 0xFF00;
            address = address | (uint16_t)value;
            cpu_registers.a = read_memory(address);
            executed_t_ticks = 12;
            break;
        }

        case 0xF1:
            //POP regAF from the stack
            cpu_registers.f = read_memory(cpu_registers.sp++) & 0xF0;
            cpu_registers.a = read_memory(cpu_registers.sp++);

            executed_t_ticks = 12;
            break;

        case 0xF2: {
            //load the contents at HIGH WRAM[regC] into regA
            uint16_t address = 0xFF00;
            address = address | (uint16_t)cpu_registers.c;
            cpu_registers.a = read_memory(address);
            executed_t_ticks = 8;
            break;
        }

        case 0xF3:
            //disable IME flag
            interrupts_enabled = false;

            executed_t_ticks = 4;
            break;

        case 0xF5:
            //PUSH regAF into the stack
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.a);
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.f & 0xF0);

            executed_t_ticks = 16;
            break;

        case 0xF6:
            //set contents of regA to regA | n8
            value = fetch();
            cpu_registers.a = or_r8(value);

            executed_t_ticks = 8;
            break;

        case 0xF7:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x30;
            executed_t_ticks = 16;
            break;

        case 0xF8:{
            //load SP + e8 into regHL
            int8_t val = (int8_t)fetch();
            uint8_t unsigned_val = (uint8_t)val;
            uint8_t sp_low = (uint8_t)(cpu_registers.sp & 0xFF);

            cpu_registers.f = 0x0;

            if ((sp_low & 0x0F )+ (unsigned_val & 0x0F) > 0x0F) cpu_registers.f |= 1 << 5;
            if (sp_low + unsigned_val > 0xFF) cpu_registers.f |= 1 << 4;

            cpu_registers.hl = (uint16_t)((int32_t)cpu_registers.sp + val);
            executed_t_ticks = 12;
            break;
        }

        case 0xF9:
            //load the contents of regHL into SP
            cpu_registers.sp = cpu_registers.hl;
            executed_t_ticks = 8;
            break;
        
        case 0xFA:{
            //load the contents at WRAM[n16] into regA
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t address = ((uint16_t)h_val<<8) | l_val;
            cpu_registers.a = read_memory(address);

            executed_t_ticks = 16;
            break;
        }

        case 0xFB:
            //enable interrupts by setting the IME flag - this should be delayed by one instruction
            ei_delay = 2;

            executed_t_ticks = 4;
            break;

        case 0xFE:
            //compare the contents of regA to n8
            value = fetch();
            comp_r8(value);

            executed_t_ticks = 8;
            break;

        case 0xFF:
            push(&cpu_registers.sp, cpu_registers.pc);
            cpu_registers.pc = 0x0038;
            executed_t_ticks = 16;
            break;


        default:
            printf("Unknown instruction!!! %4X\n", opcode);
            break;
    }

    return executed_t_ticks;
}