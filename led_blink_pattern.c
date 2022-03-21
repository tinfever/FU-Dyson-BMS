#include "main.h"
#include "led_blink_pattern.h"
#include "mcc_generated_files/epwm1.h"



void ledBlinkpattern (uint8_t num_blinks, uint8_t led_color_rgb, uint16_t blink_interval_ms, uint16_t starting_blank_time_ms, uint16_t ending_blank_time_ms, uint16_t pwm_fade_out_slope){       //Example: ledBlinkpattern(8, 0b111); would give 8 white LED blinks

    
    uint16_t timer_ms = nonblocking_wait_counter.value*32;
    uint16_t on_time = blink_interval_ms;
    uint16_t off_time = blink_interval_ms;
    
    static uint8_t max_steps = 0; 
    static uint8_t step = 0;
    static uint16_t next_step_time = 0;
    
    /*Example step schedule for num_blinks = 3, blink_interval_ms = 500, starting_blank_time_ms = 1000, ending_blank_time_ms = 1000
    *Step #     End time    LED ON
    * 0          1000,      0     //starting blank interval
    * 1          1500,      1     //blink one
    * 2          2000,      0
    * 3          2500,      1     //blink two
    * 4          3000,      0
    * 5          3500,      1     //blink three
    * 6          4000,      0
    * 7          5000,      0     //ending blank interval
     */
    
    
    //Initialize
    if (!nonblocking_wait_counter.enable){
        Set_LED_RGB(0b000, 1023);     //Turn off LED
        nonblocking_wait_counter.value = 0;
        nonblocking_wait_counter.enable = 1;
        max_steps = (2*num_blinks+2)-1;     //Subtract one so it is zero indexed
        step = 0;
        next_step_time = starting_blank_time_ms;
        if (LED_code_cycle_counter.enable){
            LED_code_cycle_counter.value++;
        }
    }
    
    if (step == 0 && timer_ms > next_step_time){                        //starting blank time
        step++;
        Set_LED_RGB(led_color_rgb, 1023);     //Turn on LED
        next_step_time += on_time;
    }
    else if (step == max_steps-1 && timer_ms > next_step_time){         //ending blank time
        step++;
        Set_LED_RGB(0b000, 1023);     //Turn off LED
        next_step_time += ending_blank_time_ms;
    }
    else if (step == max_steps && timer_ms > next_step_time){           //cycle complete
        Set_LED_RGB(0b000, 1023);     //Turn off LED
        nonblocking_wait_counter.enable = 0;
        nonblocking_wait_counter.value = 0;
    }
    else if (step % 2 != 0 && timer_ms > next_step_time){        //Step number is odd
        step++;
        Set_LED_RGB(0b000, 1023);     //Turn off LED
        next_step_time += off_time;
    }
    else if (step % 2 == 0 && timer_ms > next_step_time){       //Step number is even
        step++;
        Set_LED_RGB(led_color_rgb, 1023);     //Turn on LED
        next_step_time += on_time;
    }
    
    if (pwm_fade_out_slope > 0 && global_pwm_val > pwm_fade_out_slope){
        global_pwm_val -= pwm_fade_out_slope;
        EPWM1_LoadDutyValue(global_pwm_val);
    }
    else if (pwm_fade_out_slope > 0 && global_pwm_val <= pwm_fade_out_slope){
        global_pwm_val = 0;
        EPWM1_LoadDutyValue(global_pwm_val);
        
        nonblocking_wait_counter.value = next_step_time/32; //Shortcut to next LED step
    }
    
}

void resetLEDBlinkPattern (void){
    Set_LED_RGB(0b000, 1023);     //Turn off LED
    nonblocking_wait_counter.enable = false;
    nonblocking_wait_counter.value = 0;
    LED_code_cycle_counter.enable = false;
    LED_code_cycle_counter.value = 0;
    
}