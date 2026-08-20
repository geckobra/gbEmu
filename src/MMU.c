#include "MMU.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t nintendo_logo[48] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

uint8_t rom_memory[32768];
static uint8_t work_ram[8192];       // WRAM (0xC000 - 0xDFFF)
static uint8_t hram[127];            // HRAM (0xFF80 - 0xFFFE)
static uint8_t io_registers[128];
static uint8_t IF;
static uint8_t IE;
uint8_t sb_reg = 0;

uint8_t mmu_get_if(void) {
    return IF;
}

void mmu_set_if(uint8_t value) {
    IF = value & 0x1F;
}

uint8_t mmu_get_ie(void) {
    return IE;
}

int load_ROM(const char* path) {
    FILE* ROM = fopen(path, "rb");
    if (ROM == NULL) return -1;

    size_t bytes_read = fread(rom_memory, sizeof(uint8_t), sizeof(rom_memory), ROM);
    fclose(ROM);
    
    uint8_t logo[48];

    memcpy(logo, rom_memory+0x0104, 48);

    if (memcmp(logo, nintendo_logo, 48) == 0){
        printf("Logo looks clean!!\n");
    } else {
        printf("Warning: Nintendo Logo mismatch\n");
    }

    return (bytes_read == 0) ? -2 : 0;
}

void mem_init(void) {
    memset(rom_memory, 0, sizeof(rom_memory));
    memset(work_ram, 0, sizeof(work_ram));
    memset(hram, 0, sizeof(hram));
    memset(io_registers, 0, sizeof(io_registers));
    sb_reg = 0;
    IF = 0;
    IE = 0;
}

uint8_t read_memory(uint16_t address) {
    //ROM Region
    if (address <= 0x7FFF) {
        return rom_memory[address];
    }

    //Echo RAM mapping normalization
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    //Working RAM Region
    if (address >= WORK_RAM_START && address <= WORK_RAM_END) {
        return work_ram[address - WORK_RAM_START];
    }

    //Special Register Interceptions
    switch (address) {
        case 0xFF04: return get_DIV();
        case 0xFF05: return get_TIMA();
        case 0xFF0F: return IF;
        case 0xFFFF: return IE;
    }

    //I/O Registers Region
    if (address >= 0xFF00 && address <= 0xFF7F) {
        return io_registers[address - 0xFF00];
    }
    
    //High RAM (HRAM) Region
    if (address >= 0xFF80 && address <= 0xFFFE) {
        return hram[address - 0xFF80];
    }

    return 0x00;
}

void write_memory(uint16_t address, uint8_t value) { 
    //Special Register Side-Effects & Interceptions
    switch (address) {
        case 0xFF01:
            sb_reg = value;
            return;
        case 0xFF02:
            if (value == 0x81) {
                putchar((char)sb_reg);
                fflush(stdout);
            }
            return;
        case 0xFF04:
            reset_DIV();
            return;
        case 0xFF05:
            set_TIMA(value);
            return;
        case 0xFF0F:
            IF = value & 0x1F; //Only lower 5 bits used
            return;
        case 0xFFFF:
            IE = value;
            return;
    }

    //Echo RAM mapping normalization
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    //Working RAM Region
    if (address >= WORK_RAM_START && address <= WORK_RAM_END) {
        work_ram[address - WORK_RAM_START] = value;
        return;
    }

    //I/O Registers Region
    if (address >= 0xFF00 && address <= 0xFF7F) {
        io_registers[address - 0xFF00] = value;

        //timer config tracking
        if (address == 0xFF07 || address == 0xFF06) {
            uint8_t clk_enable  = (io_registers[0xFF07 - 0xFF00] >> 2) & 1;
            uint8_t clk_freq    = (io_registers[0xFF07 - 0xFF00] & 0x03);
            uint8_t overflow_val = io_registers[0xFF06 - 0xFF00];
            update_timer_settings(clk_enable, clk_freq, overflow_val);
        }
        return;
    }

    //High RAM (HRAM) Region
    if (address >= 0xFF80 && address <= 0xFFFE) {
        hram[address - 0xFF80] = value;
        return;
    }
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