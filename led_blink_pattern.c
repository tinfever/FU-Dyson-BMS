#include "main.h"

static uint8_t completed_half_blinks = 0;
static uint8_t LED_on = false;

void ledBlinkpattern (uint8_t num_blinks, uint8_t led_color_rgb, uint16_t blink_interval_ms){       //Example: ledBlinkpattern(8, 0b111); would give 8 white LED blinks
    
    
    const uint16_t starting_blank_time = 2000;
    
    uint16_t timer_ms = nonblocking_wait_counter.value*32;
    
    if (!nonblocking_wait_counter.enable){
        Set_LED_RGB(0b000);     //Turn off LED
        nonblocking_wait_counter.value = 0;
        nonblocking_wait_counter.enable = 1;
    }
    
    if (timer_ms >= (completed_half_blinks*blink_interval_ms)+starting_blank_time && timer_ms < ((completed_half_blinks+1)*blink_interval_ms)+starting_blank_time){
        if (LED_on){
            Set_LED_RGB(led_color_rgb);
        }
        else{
            Set_LED_RGB(0b000);     //LED off
        }
    }
    else if (timer_ms >= ((completed_half_blinks+1)*blink_interval_ms)+starting_blank_time){
        completed_half_blinks++;
        LED_on = !LED_on;
    }
    
    if (completed_half_blinks >= num_blinks*2){
        Set_LED_RGB(0b000);     //Turn off LED
        nonblocking_wait_counter.enable = 0;
        nonblocking_wait_counter.value = 0;
        completed_half_blinks = 0;
        LED_on = false;
    }
}

void resetLEDBlinkPattern (void){
    Set_LED_RGB(0b000);     //Turn off LED
    nonblocking_wait_counter.enable = 0;
    nonblocking_wait_counter.value = 0;
    completed_half_blinks = 0;
    LED_on = false;
    
}