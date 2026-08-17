#include "MMU.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t rom_memory[32768];
static uint8_t work_ram[8192]; // WRAM (0xC000 - 0xDFFF)
static uint8_t hram[127];      // HRAM (0xFF80 - 0xFFFE)
static uint8_t io_registers[128];
static uint8_t IE;
static uint8_t IF;
uint8_t sb_reg = 0;

static void print_rom_contents(const uint8_t* rom, uint16_t start_addr, uint16_t end_addr) {
    for (uint32_t i = start_addr; i <= end_addr; i += 16) {
        printf("0x%04X:  ", i);
        for (uint32_t j = 0; j < 16; j++) {
            if (i + j <= end_addr) {
                printf("%02X ", rom[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (uint32_t j = 0; j < 16; j++) {
            if (i + j <= end_addr) {
                uint8_t byte = rom[i + j];
                printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
            }
        }
        printf("\n");
    }
}

int load_ROM(const char* path) {
    FILE* ROM = fopen(path, "rb");
    if (ROM == NULL) {
        return -1;
    }

    size_t bytes_read = fread(rom_memory, sizeof(uint8_t), sizeof(rom_memory), ROM);
    fclose(ROM);

    if (bytes_read == 0) {
        return -2;
    }

    print_rom_contents(rom_memory, 0x0100, 0x7FFF);
    return 0;
}

void mem_init(void) {
    memset(rom_memory, 0, sizeof(rom_memory));
    memset(work_ram, 0, sizeof(work_ram));
    memset(hram, 0, sizeof(hram));
    sb_reg = 0;
}

uint8_t read_memory(uint16_t address) {

    if (address <= 0x7FFF) {
        return rom_memory[address];
    }

    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000; //Translate Echo RAM to WRAM
    }

    if (address >= WORK_RAM_START && address <= WORK_RAM_END) {
        return work_ram[address - WORK_RAM_START];
    }

    if (address >= 0xFF00 && address <= 0xFF7F) {
        return io_registers[address - 0xFF00];
    }
    
    if (address >= 0xFF80 && address <= 0xFFFE) {
        return hram[address - 0xFF80];
    }

    if (address == 0xFF0F) return IF;
    if (address == 0xFFFF) return IE;

    return 0x00;
}

void write_memory(uint16_t address, uint8_t value) {

    if (address == 0xFF01) {
        sb_reg = value;
        return;
    } 
    if (address == 0xFF02) {
        if (value == 0x81) {
            char c = (char)sb_reg;
            printf("%c", c);
            fflush(stdout);

            value = 0x01;
        }
        return;
    }

    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    if (address >= WORK_RAM_START && address <= WORK_RAM_END) {
        work_ram[address - WORK_RAM_START] = value;
        return;
    }

    if (address >= 0xFF00 && address <= 0xFF7F) {
        io_registers[address - 0xFF00] = value;
        return;
    }

    if (address >= 0xFF80 && address <= 0xFFFE) {
        hram[address - 0xFF80] = value;
        return;
    }

    if (address == 0xFF0F) IF = value;
    if (address == 0xFFFF) IE = value;
}

void push(uint16_t* sp, uint16_t val) {
    uint8_t low = val & 0xFF;
    uint8_t high = (val >> 8) & 0xFF;

    *sp -= 1;
    write_memory(*sp, high);
    *sp -= 1;
    write_memory(*sp, low);
}

uint16_t pop(uint16_t* sp) {
    uint8_t low = read_memory(*sp);
    *sp += 1;
    uint8_t high = read_memory(*sp);
    *sp += 1;

    return ((uint16_t)high << 8) | low;
}