//this file is a generic implementation of the Memory Management Unit used by the original GameBoy
#include <stdint.h>

#define WORK_RAM_START 0xC000
#define WORK_RAM_END   0xDFFF
#define ECHO_RAM_START 0xE000
#define ECHO_RAM_END   0xFDFF

uint8_t read_memory(uint16_t);
void write_memory(uint16_t, uint8_t);

uint16_t pop(uint16_t*);
uint16_t push(uint16_t*, uint16_t);