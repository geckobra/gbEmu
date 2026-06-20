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
    uint16_t st;
} sm83_registers_t;

extern sm83_registers_t cpu_registers;
extern bool isHalted;

void cpu_init();
void run_cpu();
int execute(uint8_t);
uint8_t fetch(uint16_t);