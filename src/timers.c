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
    uint8_t clk_IR_value;
} TIMA_cfg_t;

TIMA_cfg_t timer_cfg;
timer_data_t timers;

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
    //read TIMA clock settings before ticking it
    uint8_t TAC_val = read_memory(0xFF07);

    timers.div_cycles += (t_cycles / 4);

    //update TIMA settings
    timer_cfg.clk_enabled = (TAC_val >> 2) & 1;
    timer_cfg.clk_freq = (TAC_val) & 0x03;

    if (timers.div_cycles > 64){
        timers.DIV++;
        timers.div_cycles = 0;
    }

    if (timer_cfg.clk_enabled){
        timers.tima_cycles += (t_cycles / 4);

        if (timers.tima_cycles > TIMA_FREQS[timer_cfg.clk_freq]){
            last_TIMA_val = timers.TIMA++;
            timers.tima_cycles = 0;
        }

        if (timers.TIMA == 0x00 && last_TIMA_val == 0xFF){
            timers.TIMA = read_memory(0xFF06); //set value to that of the Timer Modulo
            set_timer_IR();
        }
    }
}