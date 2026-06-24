#include "CPU.h"
#include "MMU.h"

//returns the T-Cycles needed to run the instruction and advances the PC as needed
int execute(uint8_t opcode){

    int executed_t_ticks = 0;
    uint8_t value = 0; //8 bit value to extract from the instruction
    
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

        case 0x06:
            //load n8 value into regB
            value = fetch();
            printf("Load %d into B\n", value);

            cpu_registers.b = value;
            
            executed_t_ticks = 8;
            break;

        case 0x07:
            //perform left circular rotation on regA
            cpu_registers.a = rlc_r8(cpu_registers.a);

            cpu_registers.f &= 0b1011111; //this instruction resets zero flag (bit 7)

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

        case 0x0A:
            //load in regA the value stored at WRAM[BC]
            cpu_registers.a = read_memory(cpu_registers.bc);
            executed_t_ticks = 8;
            break;

        case 0x0C:
            //increment regC and set corresponding flags
            cpu_registers.c = inc_r8(cpu_registers.c);
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

            cpu_registers.f &= 0b1011111; //this instruction resets zero flag (bit 7)

            executed_t_ticks = 4;
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

        case 0x16:
            //load an 8 bit value in regD
            value = fetch();
            cpu_registers.d = value;
            executed_t_ticks = 8;
            break;

        case 0x17:
            //perform left rotation through carry flag on regA
            cpu_registers.a = rl_r8(cpu_registers.a);

            cpu_registers.f &= 0b10111111;

            executed_t_ticks = 4;
            break;
        
        case 0x1A:
            //load value at WRAM[DE] into regA
            cpu_registers.a = read_memory(cpu_registers.de);
            executed_t_ticks = 8;
            break;

        case 0x1C:
            //increment regE and set corresponding flags
            cpu_registers.e = inc_r8(cpu_registers.e);
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

            cpu_registers.f &= 0b10111111;
            executed_t_ticks = 4;
            break;

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

        case 0x26:
            //load n8 value into regH
            value = fetch();
            cpu_registers.h = value;
            executed_t_ticks = 8;
            break;

        case 0x2A:
            //load WRAM[regHL] into regA and increment regHL by 1
            cpu_registers.a = read_memory(cpu_registers.hl);
            cpu_registers.hl++;
            executed_t_ticks = 8;
            break;

        case 0x2C:
            //increment regL and set corresponding flags
            cpu_registers.l = inc_r8(cpu_registers.l);
            executed_t_ticks = 4;
            break;

        case 0x2E:
            //load n8 value into regL
            value = fetch();
            cpu_registers.l = value;
            executed_t_ticks = 8;
            break;

        case 0x2F:
            //complement accumulator
            cpu_registers.a = not();

            executed_t_ticks = 4;
            break;

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

        case 0x34:
            //increment WRAM[regHL] and set corresponding flags
            uint8_t value = read_memory(cpu_registers.hl);
            value = inc_r8(value);
            write_memory(cpu_registers.hl, value);

            executed_t_ticks = 12;
            break;

        case 0x36:
            //write n8 value into WRAM[regHL]
            value = fetch();
            write_memory(cpu_registers.hl, value);
            executed_t_ticks  = 12;
            break;

        case 0x37:
            //set carry flag

            cpu_registers.f &= 0b11001111; //reset H and N flags keeping Zero and Carry flags
            cpu_registers.f |= 1 << 4;     //set carry flag afterwards

            executed_t_ticks = 4;
            break;

        case 0x3A:
            //load value at WRAM[regHL] into regA and decrement HL by 1
            cpu_registers.a = read_memory(cpu_registers.hl);
            cpu_registers.hl--;
            executed_t_ticks = 8;
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

        case 0x3F:
            //complement carry flag

            cpu_registers.f &= 0b11001111; //reset H and N flags keeping Zero and Carry flags
            cpu_registers.f ^= 1 << 4;     //invert carry flag

            executed_t_ticks = 4;
            break;
    
        case 0x40:
            //load regB into regB

            //load operation is not needed, just tick the PPU and APU for 4T-C
            executed_t_ticks = 4;
            break;

        case 0x41:
            //load the contents of regC into regB
            cpu_registers.b = cpu_registers.c;
            executed_t_ticks = 4;
            break;

        case 0x42:
            //load the contents of regD into regB
            cpu_registers.b = cpu_registers.d;
            executed_t_ticks = 4;
            break;

        case 0x43:
            //load the contents of regE into regB
            cpu_registers.b = cpu_registers.e;
            executed_t_ticks = 4;
            break;

        case 0x44:
            //load the contents of regH into regB
            cpu_registers.b = cpu_registers.h;
            executed_t_ticks = 4;
            break;

        case 0x45:
            //load the contents of regL into regB
            cpu_registers.b = cpu_registers.l;
            executed_t_ticks = 4;
            break;

        case 0x46:
            //load the value at WRAM[HL] into regB
            cpu_registers.b = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;

        case 0x47:
            //load the contents of regA into regB
            cpu_registers.b = cpu_registers.a;
            executed_t_ticks = 4;
            break;

        case 0x48:
            //load the contents of regB into regC
            cpu_registers.c = cpu_registers.b;
            executed_t_ticks = 4;
            break;

        case 0x49:
            //load regC into regC
            executed_t_ticks = 4; //tick PPU and APU for 4T-C
            break;

        case 0x4A:
            //load the contents of regD into regC
            cpu_registers.c = cpu_registers.d;
            executed_t_ticks = 4;
            break;

        case 0x4B:
            //load the contents of regE into regC
            cpu_registers.c = cpu_registers.e;
            executed_t_ticks = 4;
            break;

        case 0x4C:
            //load the contents of regH into regC
            cpu_registers.c = cpu_registers.h;
            executed_t_ticks = 4;
            break;

        case 0x4D:
            //load the contents of regL into regC
            cpu_registers.c = cpu_registers.l;
            executed_t_ticks = 4;
            break;

        case 0x4E:
            //load the contents of WRAM[HL] into regC
            cpu_registers.c = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;

        case 0x4F:
            //load the contents of regA into regC
            cpu_registers.c = cpu_registers.a;
            executed_t_ticks = 4;
            break;

        case 0x50:
            //load the contents of regB into regD
            cpu_registers.d = cpu_registers.b;
            executed_t_ticks = 4;
            break;

        case 0x51:
            //load the contents of regC into regD
            cpu_registers.d = cpu_registers.c;
            executed_t_ticks = 4;
            break;

        case 0x52:
            //load the contents of regD into regD

            executed_t_ticks = 4;
            break;

        case 0x53:
            //load the contents of regE into regD
            cpu_registers.d = cpu_registers.e;
            executed_t_ticks = 4;
            break;

        case 0x54:
            //load the contents of regH into regD
            cpu_registers.d = cpu_registers.h;
            executed_t_ticks = 4;
            break;

        case 0x55:
            //load the contentes of regL into regD
            cpu_registers.d = cpu_registers.l;
            executed_t_ticks = 4;
            break;

        case 0x56:
            //load the value at WRAM[HL] into regD
            cpu_registers.d = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;
        
        case 0x57:
            //load the value at regA into regD
            cpu_registers.d = cpu_registers.a;
            executed_t_ticks = 4;
            break;

        case 0x58:
            //load the value at regB into regE
            cpu_registers.e = cpu_registers.b;
            executed_t_ticks = 4;
            break;

        case 0x59:
            //load the value at regC into regE
            cpu_registers.e = cpu_registers.c;
            executed_t_ticks = 4;
            break;

        case 0x5A:
            //load the value at regD into regE
            cpu_registers.e = cpu_registers.d;
            executed_t_ticks = 4;
            break;

        case 0x5B:
            //load the vlaue at regE into regE

            executed_t_ticks = 4;
            break;

        case 0x5C:
            //load the value at regH into regE
            cpu_registers.e = cpu_registers.h;
            executed_t_ticks = 4;
            break;
        
        case 0x5D:
            //load the value at regL into regE
            cpu_registers.e = cpu_registers.l;
            executed_t_ticks = 4;
            break;
        
        case 0x5E:
            //load the value at WRAM[HL] into regE
            cpu_registers.e = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;

        case 0x5F:
            //load the contents of regA into regE
            cpu_registers.e = cpu_registers.a;
            executed_t_ticks = 4;
            break;

        case 0x60:
            //load the contents of regB into regH
            cpu_registers.h = cpu_registers.b;
            executed_t_ticks = 4;
            break;

        case 0x61:
            //load the contents of regC into regH
            cpu_registers.h = cpu_registers.c;
            executed_t_ticks = 4;
            break;

        case 0x62:
            //load the contents of regD into regH
            cpu_registers.h = cpu_registers.d;
            executed_t_ticks = 4;
            break;

        case 0x63:
            //load the contents of regE into regH
            cpu_registers.h = cpu_registers.e;
            executed_t_ticks = 4;
            break;

        case 0x64:
            //load the contents of regH into regH

            executed_t_ticks = 4;
            break;

        case 0x65:
            //load the contents of regL into regH
            cpu_registers.h = cpu_registers.l;
            executed_t_ticks = 4;
            break;

        case 0x66:
            //load the value at WRAM[HL] into regH
            cpu_registers.h = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;

        case 0x67:
            //load the contents of regA into regH
            cpu_registers.h = cpu_registers.a;
            executed_t_ticks = 4;
            break;

        case 0x68:
            //load the contentes of regB into regL
            cpu_registers.l = cpu_registers.b;
            executed_t_ticks = 4;
            break;

        case 0x69:
            //load the contents of regC into regL
            cpu_registers.l = cpu_registers.c;
            executed_t_ticks = 4;
            break;

        case 0x6A:
            //load the contents of regD into regL
            cpu_registers.l = cpu_registers.d;
            executed_t_ticks = 4;
            break;

        case 0x6B:
            //load the contents of regE into regL
            cpu_registers.l = cpu_registers.e;
            executed_t_ticks = 4;
            break;

        case 0x6C:
            //load the contents of regH into regL
            cpu_registers.l = cpu_registers.h;
            executed_t_ticks = 4;
            break;

        case 0x6D:
            //load the contents of regL into regL
            executed_t_ticks = 4;
            break;

        case 0x6E:
            //load the value at WRAM[HL] into regL
            cpu_registers.l = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;

        case 0x6F:
            //load the contents of regA into regL
            cpu_registers.l = cpu_registers.a;
            executed_t_ticks = 4;
            break;

        case 0x70:
            //write the contents of regB into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.b);
            executed_t_ticks = 8;
            break;

        case 0x71:
            //write the contentes of regC into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.c);
            executed_t_ticks = 8;
            break;

        case 0x72:
            //write the contents of regD into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.d);
            executed_t_ticks = 8;
            break;

        case 0x73:
            //write the contentes of regE into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.e);
            executed_t_ticks = 8;
            break;

        case 0x74:
            //write the contents of regH into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.h);
            executed_t_ticks = 8;
            break;

        case 0x75:
            //write the contents of regL into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.l);
            executed_t_ticks = 8;
            break;

        case 0x76:
            //HALTs the CPU until an interrupt is received
            isHalted = true;
            executed_t_ticks = 4;
            break;

        case 0x77:
            //write the contents of regA into WRAM[HL]
            write_memory(cpu_registers.hl, cpu_registers.a);
            executed_t_ticks = 8;
            break;
        
        case 0x78:
            //load the contentes of regB into regA
            cpu_registers.a = cpu_registers.b;
            executed_t_ticks = 4;
            break;

        case 0x79:
            //load the contents of regC into regA
            cpu_registers.a = cpu_registers.c;
            executed_t_ticks = 4;
            break;

        case 0x7A:
            //load the contents of regD into regA
            cpu_registers.a = cpu_registers.d;
            executed_t_ticks = 4;
            break;

        case 0x7B:
            //load the contents of regE into regA
            cpu_registers.a = cpu_registers.e;
            executed_t_ticks = 4;
            break;

        case 0x7C:
            //load the contents of regH into regA
            cpu_registers.a = cpu_registers.h;
            executed_t_ticks = 4;
            break;

        case 0x7D:
            //load the contents of regL into regA
            cpu_registers.a = cpu_registers.l;
            executed_t_ticks = 4;
            break;

        case 0x7E:
            //load the value at WRAM[HL] into regA
            cpu_registers.a = read_memory(cpu_registers.hl);
            executed_t_ticks = 8;
            break;

        case 0x7F:
            //load the contents of regA into regA
            executed_t_ticks = 4;
            break;

        case 0x80:
            //add contents of regB to regA
            cpu_registers.a = alu_add8(cpu_registers.b);
            executed_t_ticks = 4;
            break;

        case 0x81:
            //add contents of regC to regA
            cpu_registers.a = alu_add8(cpu_registers.c);
            executed_t_ticks = 4;
            break;

        case 0x82:
            //add contents of regD to regA
            cpu_registers.a = alu_add8(cpu_registers.d);
            executed_t_ticks = 4;
            break;

        case 0x83:
            //add contents of regE to regA
            cpu_registers.a = alu_add8(cpu_registers.e);
            executed_t_ticks = 4;
            break;

        case 0x84:
            //add contents of regH to regA
            cpu_registers.a = alu_add8(cpu_registers.h);
            executed_t_ticks = 4;
            break;

        case 0x85:
            //add contents of regL to regA
            cpu_registers.a = alu_add8(cpu_registers.l);
            executed_t_ticks = 4;
            break;

        case 0x86:
            //add contents of WRAM[regHL] to regA
            cpu_registers.a = alu_add8(read_memory(cpu_registers.hl));
            executed_t_ticks = 8;
            break;

        case 0x87:
            //add contents of regA to regA
            cpu_registers.a = alu_add8(cpu_registers.a);
            executed_t_ticks = 4;
            break;

        case 0x88:
            //add contents of regB to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.b);
            executed_t_ticks = 4;
            break;

        case 0x89:
            //add contents of regC to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.c);
            executed_t_ticks = 4;
            break;

        case 0x8A:
            //add contents of regD to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.d);
            executed_t_ticks = 4;
            break;

        case 0x8B:
            //add contents of regE to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.e);
            executed_t_ticks = 4;
            break;

        case 0x8C:
            //add contents of regH to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.h),
            executed_t_ticks = 4;
            break;

        case 0x8D:
            //add contents of regL to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.l);
            executed_t_ticks = 4;
            break;

        case 0x8E:
            //add contents of WRAM[regHL] to regA
            cpu_registers.a = alu_adc8(read_memory(cpu_registers.hl));
            executed_t_ticks = 8;
            break;

        case 0x8F:
            //add contents of regA to regA with carry
            cpu_registers.a = alu_adc8(cpu_registers.a);
            executed_t_ticks = 4;
            break;

        case 0x90:
            //substract contents of regB from regA
            cpu_registers.a = alu_sub8(cpu_registers.b);
            executed_t_ticks = 4;
            break;

        case 0x91:
            //substract contents of regC from regA
            cpu_registers.a = alu_sub8(cpu_registers.c);
            executed_t_ticks = 4;
            break;

        case 0x92:
            //substract contents of regD from regA
            cpu_registers.a = alu_sub8(cpu_registers.d);
            executed_t_ticks = 4;
            break;

        case 0x93:
            //substract contents of regE from regA
            cpu_registers.a = alu_sub8(cpu_registers.e);
            executed_t_ticks = 4;
            break;

        case 0x94:
            //substract contents of regH from regA
            cpu_registers.a = alu_sub8(cpu_registers.h);
            executed_t_ticks = 4;
            break;

        case 0x95:
            //substract contents of regL from regA
            cpu_registers.a = alu_sub8(cpu_registers.l);
            executed_t_ticks = 4;
            break;

        case 0x96:{
            //substract contents of WRAM[regHL] from regA
            uint8_t value = read_memory(cpu_registers.hl);
            value = alu_sub8(value);
            cpu_registers.a = value;

            executed_t_ticks = 8;
            break;
        }

        case 0x97:
            //susbtract contents of regA from regA
            cpu_registers.a = alu_sub8(cpu_registers.a);
            
            executed_t_ticks = 4;
            break;

        case 0x98:
            //substract with carry the contents of regB from regA
            cpu_registers.a = alu_subc8(cpu_registers.b);

            executed_t_ticks = 4;
            break;

        case 0x99:
            //substract with carry the contents of regC from regA
            cpu_registers.a = alu_subc8(cpu_registers.c);

            executed_t_ticks = 4;
            break;

        case 0x9A:
            //substract with carry the contents of regD from regA
            cpu_registers.a = alu_subc8(cpu_registers.d);

            executed_t_ticks = 4;
            break;

        case 0x9B:
            //substract with carry the contents of regE from regA
            cpu_registers.a = alu_subc8(cpu_registers.e);

            executed_t_ticks = 4;
            break;

        case 0x9C:
            //substract with carry the contents of regH from regA
            cpu_registers.a = alu_subc8(cpu_registers.h);
            
            executed_t_ticks = 4;
            break;

        case 0x9D:
            //substract with carry the contents of regL from regA
            cpu_registers.a = alu_subc8(cpu_registers.l);

            executed_t_ticks = 4;
            break;

        case 0x9E:{
            //substract with carry the contents of WRAM[regHL] from regA
            uint8_t value = read_memory(cpu_registers.hl);
            cpu_registers.a= alu_subc8(value);

            executed_t_ticks = 8;
            break;
        }

        case 0x9F:
            //substract with carry the contents of regA from regA
            cpu_registers.a = alu_subc8(cpu_registers.a);

            executed_t_ticks = 4;
            break;

        case 0xA0:
            //set contents of regA to regA & regB
            cpu_registers.a = and_r8(cpu_registers.b);

            executed_t_ticks = 4;
            break;

        case 0xA1:
            //set contents of regA to regA & regC
            cpu_registers.a = and_r8(cpu_registers.c);

            executed_t_ticks = 4;
            break;

        case 0xA2:
            //set contents of regA to regA & regD
            cpu_registers.a = and_r8(cpu_registers.d);

            executed_t_ticks = 4;
            break;

        case 0xA3:
            //set contents of regA to regA & regE
            cpu_registers.a = and_r8(cpu_registers.e);

            executed_t_ticks = 4;
            break;

        case 0xA4:
            //set contents of regA to regA & regH
            cpu_registers.a = and_r8(cpu_registers.h);

            executed_t_ticks = 4;
            break;

        case 0xA5:
            //set contents of regA to regA & regL
            cpu_registers.a = and_r8(cpu_registers.l);

            executed_t_ticks = 4;
            break;

        case 0xA6:
            //set contents of regA to regA & WRAM[regHL]
            value = read_memory(cpu_registers.hl);
            cpu_registers.a = and_r8(value);
            
            executed_t_ticks = 8;
            break;

        case 0xA7:
            //seet contents of regA to regA & regA
            cpu_registers.a = and_r8(cpu_registers.a);

            executed_t_ticks = 4;
            break;

        case 0xA8:
            //set contentes of regA to regA ^ regB
            cpu_registers.a = xor_r8(cpu_registers.b);

            executed_t_ticks = 4;
            break;

        case 0xA9:
            //set contentes of regA to regA ^ regC
            cpu_registers.a = xor_r8(cpu_registers.c);

            executed_t_ticks = 4;
            break;

        case 0xAA:
            //set contents of regA to regA ^ regD
            cpu_registers.a = xor_r8(cpu_registers.d);
            
            executed_t_ticks = 4;
            break;

        case 0xAB:
            //set contents of regA to regA ^ regE
            cpu_registers.a = xor_r8(cpu_registers.e);

            executed_t_ticks = 4;
            break;

        case 0xAC:
            //set contents of regA to regA ^ regH
            cpu_registers.a = xor_r8(cpu_registers.h);

            executed_t_ticks = 4;
            break;

        case 0xAD:
            //set the contents of regA to regA ^ regL
            cpu_registers.a = xor_r8(cpu_registers.l);

            executed_t_ticks = 4;
            break;

        case 0xAE:
            //set the contents of regA to regA ^ WRAM[regHL]
            cpu_registers.a = xor_r8(read_memory(cpu_registers.hl));

            executed_t_ticks = 8;
            break;

        case 0xAF:
            //set the contents of regA to regA ^ regA
            cpu_registers.a = xor_r8(cpu_registers.a);

            executed_t_ticks = 4;
            break;

        case 0xB0:
            //set the contents of regA to regA | regB
            cpu_registers.a = or_r8(cpu_registers.b);
            
            executed_t_ticks = 4;
            break;

        case 0xB1:
            //set the contents of regA to regA | regC
            cpu_registers.a = or_r8(cpu_registers.c);

            executed_t_ticks = 4;
            break;

        case 0xB2:
            //set the contents of regA to regA | regD
            cpu_registers.a = or_r8(cpu_registers.d);

            executed_t_ticks = 4;
            break;

        case 0xB3:
            //set the contents of regA to regA | regE
            cpu_registers.a = or_r8(cpu_registers.e);

            executed_t_ticks = 4;
            break;

        case 0xB4:
            //set the contents of regA to regA | regH
            cpu_registers.a = or_r8(cpu_registers.h);

            executed_t_ticks = 4;
            break;

        case 0xB5:
            //set the contents of regA to regA | regL
            cpu_registers.a = or_r8(cpu_registers.l);

            executed_t_ticks = 4;
            break;

        case 0xB6:
            //set the contents of regA to regA | WRAM[regHL]
            cpu_registers.a = or_r8(read_memory(cpu_registers.hl));

            executed_t_ticks = 8;
            break;

        case 0xB7:
            //set the contents of regA to regA | regA
            cpu_registers.a = or_r8(cpu_registers.a);

            executed_t_ticks = 4;
            break;

        case 0xB8:
            //compare the contents of regA with regB
            comp_r8(cpu_registers.b);

            executed_t_ticks = 4;
            break;

        case 0xB9:
            //compare the contents of regA with regC
            comp_r8(cpu_registers.c);

            executed_t_ticks = 4;
            break;

        case 0xBA:
            //compare the contents of regA with regD
            comp_r8(cpu_registers.d);

            executed_t_ticks = 4;
            break;

        case 0xBB:
            //compare the contents of regA with regE
            comp_r8(cpu_registers.e);

            executed_t_ticks = 4;
            break;

        case 0xBC:
            //compare the contents of regA with regH
            comp_r8(cpu_registers.h);

            executed_t_ticks = 4;
            break;

        case 0xBD:
            //compare the contents of regA with regL
            comp_r8(cpu_registers.l);

            executed_t_ticks = 4;
            break;

        case 0xBE:
            //compare the contents of regA with WRAM[regHL]
            comp_r8(read_memory(cpu_registers.hl));
            
            executed_t_ticks = 8;
            break;

        case 0xBF:
            //compare the contents of regA with regA
            comp_r8(cpu_registers.a);

            executed_t_ticks = 4;
            break;

        case 0xC1:
            //POP 16 bits from the stack and store at BC
            cpu_registers.c = read_memory(cpu_registers.sp++);
            cpu_registers.b = read_memory(cpu_registers.sp++); //INC SP so it points to the top of the stack

            executed_t_ticks = 12;
            break;

        case 0xC5:
            //PUSH regBC into the stack
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.b); //write high byte first
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.c); //write low byte second, Little-Endian

            executed_t_ticks = 16;
            break;

        case 0xC6:
            //add n8 to regA
            value = fetch();
            cpu_registers.a = alu_add8(value);

            executed_t_ticks = 8;
            break;

        case 0xCE:
            //add n8 to regA with carry
            value = fetch();
            cpu_registers.a = alu_adc8(value);
            
            executed_t_ticks = 8;
            break;

        case 0xD1:
            //POP 16 bits from the stack and store at DE
            cpu_registers.e = read_memory(cpu_registers.sp++);
            cpu_registers.d = read_memory(cpu_registers.sp++);

            executed_t_ticks = 12;
            break;

        case 0xD5:
            //PUSH regDE into the stack
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.d);
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.e);

            executed_t_ticks = 16;
            break;

        case 0xD6:
            //substract n8 from regA
            value = fetch();
            cpu_registers.a = alu_sub8(value);

            executed_t_ticks = 8;
            break;

        case 0xDE:  
            //substract n8 from regA with carry
            value = fetch();
            cpu_registers.a = alu_subc8(value);

            executed_t_ticks = 8;
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

    }

    return executed_t_ticks;
}