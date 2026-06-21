#pragma once
//This file contains the interface for the gameboy CPU implementation, which resides in CPU.c

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct{
    union { struct { uint8_t f; uint8_t a;}; uint16_t af;};
    union { struct { uint8_t c; uint8_t b;}; uint16_t bc;};
    union { struct { uint8_t e; uint8_t d;}; uint16_t de;};
    union { struct { uint8_t l; uint8_t h;}; uint16_t hl;};

    uint16_t pc;
    uint16_t sp;
} sm83_registers_t;

extern sm83_registers_t cpu_registers;
extern bool isHalted;

//ALU HELPER FUNCTIONS
uint8_t alu_add8(uint8_t); //performs addition to the accumulation register and sets corresponding flags
uint8_t alu_adc8(uint8_t); //performs addition with carry to the accumulation register and sets corresponding flags
uint8_t alu_sub8(uint8_t); //performs substraction to the accumulation register and sets corresopnding flags
uint8_t alu_subc8(uint8_t); //performs substraction with carry to the accumulation register and seets corresponding flags
uint8_t    inc_r8(uint8_t);
uint8_t    inc_r16(uint16_t);

//BITWISE LOGIC INSTRUCTIONS
uint8_t and_r8(uint8_t); //set regA to bitwise AND with r8


void cpu_init();
void run_cpu();
int execute(uint8_t);
uint8_t fetch();