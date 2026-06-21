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

        case 0x06:
            //load n8 value into regB
            value = fetch();
            printf("Load %d into B\n", value);

            cpu_registers.b = value;
            
            executed_t_ticks = 8;
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

        case 0x0E:
            //load an 8 bit value in regC
            value = fetch();
            cpu_registers.c = value;
            executed_t_ticks = 8;
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

        case 0x16:
            //load an 8 bit value in regD
            value = fetch();
            cpu_registers.d = value;
            executed_t_ticks = 8;
            break;
        
        case 0x1A:
            //load value at WRAM[DE] into regA
            cpu_registers.a = read_memory(cpu_registers.de);
            executed_t_ticks = 8;
            break;

        case 0x1E:
            //load 8 bit value into regE
            value = fetch();
            cpu_registers.e = value;
            executed_t_ticks = 8;
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

        case 0x2E:
            //load n8 value into regL
            value = fetch();
            cpu_registers.l = value;
            executed_t_ticks = 8;
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

        case 0x36:
            //write n8 value into WRAM[regHL]
            value = fetch();
            write_memory(cpu_registers.hl, value);
            executed_t_ticks  = 12;
            break;

        case 0x3A:
            //load value at WRAM[regHL] into regA and decrement HL by 1
            cpu_registers.a = read_memory(cpu_registers.hl);
            cpu_registers.hl--;
            executed_t_ticks = 8;
            break;

        case 0x3E:
            //load n8 value into regA
            value = fetch();
            cpu_registers.a = value;
            executed_t_ticks = 8;
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

        case 0xEA:{
            //write the contents of regA to WRAM[n16]
            uint8_t l_val = fetch();
            uint8_t h_val = fetch();
            uint16_t res_address = ((uint16_t)h_val << 8) | l_val;
            write_memory(res_address, cpu_registers.a);
            executed_t_ticks = 16;
            break;
        }

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

        case 0xF5:
            //PUSH regAF into the stack
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.a);
            cpu_registers.sp--;
            write_memory(cpu_registers.sp, cpu_registers.f & 0xF0);

            executed_t_ticks = 16;
            break;

        case 0xF8:{
            //TODO: Implement check for H and C flags

            //load SP + e8 into regHL
            int8_t val = (int8_t)fetch();
            cpu_registers.hl = (cpu_registers.sp + (int16_t)val);
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

    }

    return executed_t_ticks;
}