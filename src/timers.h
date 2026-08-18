#include "CPU.h"

void timers_init();
void tick_timers(uint8_t);
void update_timer_settings(uint8_t, uint8_t, uint8_t);
uint8_t get_DIV();
uint8_t get_TIMA();
void reset_DIV();
void set_TIMA(uint8_t);