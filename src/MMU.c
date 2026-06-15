#include "MMU.h"

static uint8_t work_ram[8192]; //two regions of work RAM, 4KiB each

uint8_t read_memory(uint16_t address){
    //returns the value stored at the requested address

    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000; //tanslate the address inside ECHO RAM into WORK RAM
    }

    //if the address is outside the work RAM, return zero
    if (address >= WORK_RAM_START && address <= WORK_RAM_END) {
        //to get the location inside the standalone array, substract the starting WRAM offset as to align
        //the address to 'zero'
        return work_ram[address - WORK_RAM_START];
    }

    return 0x00;
}

void write_memory(uint16_t address, uint8_t value){
    //write the value to the specified address

    //first, normalize the adddress if it is in ERAM
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x200;
    }

    //then check if the address is within WRAM and write the value
    if (address >= WORK_RAM_START && address <= WORK_RAM_END) {
        work_ram[address - WORK_RAM_START] = value;
    }
}