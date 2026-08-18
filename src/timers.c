#include "MMU.h"
#include "timers.h"

static const uint16_t TIMA_FREQS[] = {256, 4, 16, 64}; //all possible TIMA frequencies ordered so they can be indexed by TAC
static uint8_t last_TIMA_val;

typedef struct{
    uint8_t DIV;
    uint8_t TIMA;
    uint16_t div_cycles;  //M-cycles since the last time DIV has been incremented
    uint16_t tima_cycles; //M-cycles since the last time TIMA has been incremented
} timer_data_t;

typedef struct{
    uint8_t clk_enabled;
    uint8_t clk_freq;
    uint8_t clk_IR_value; //value to set TIMA after an overflow
} TIMA_cfg_t;

TIMA_cfg_t timer_cfg;
timer_data_t timers;

void set_TIMA(uint8_t value) {
    timers.TIMA = value;
}

uint8_t get_DIV() {
    return timers.DIV;
}

uint8_t get_TIMA() {
    return timers.TIMA;
}

void reset_DIV() {
    timers.DIV = 0;
    timers.div_cycles = 0;
}

void update_timer_settings(uint8_t enable, uint8_t freq, uint8_t IR_value){
    timer_cfg.clk_enabled = enable;
    timer_cfg.clk_freq = freq;
    timer_cfg.clk_IR_value = IR_value;
}

void timers_init(){
    timer_cfg.clk_enabled = 0;
    timer_cfg.clk_freq = 0x00;
    timer_cfg.clk_IR_value = 0xFF;

    timers.DIV = 0x00;
    timers.TIMA = 0x00;
    last_TIMA_val = timers.TIMA;
    timers.div_cycles = 0;
    timers.tima_cycles = 0;
}

void tick_timers(uint8_t t_cycles){
    uint8_t m_cycles = t_cycles / 4;
    if (m_cycles == 0) m_cycles = 1;

    timers.div_cycles += m_cycles;
    while (timers.div_cycles >= 64) {
        timers.DIV++;
        timers.div_cycles -= 64;
    }

    if (timer_cfg.clk_enabled){
        timers.tima_cycles += m_cycles;
        
        uint16_t freq_limit = TIMA_FREQS[timer_cfg.clk_freq];
        while (timers.tima_cycles >= freq_limit) {
            timers.tima_cycles -= freq_limit;
            
            last_TIMA_val = timers.TIMA;
            timers.TIMA++;

            if (timers.TIMA == 0x00 && last_TIMA_val == 0xFF){
                timers.TIMA = timer_cfg.clk_IR_value; // Timer Modulo
                uint8_t IF = mmu_get_if();
                mmu_set_if(IF | (1 << 2));
            }
        }
    }
}