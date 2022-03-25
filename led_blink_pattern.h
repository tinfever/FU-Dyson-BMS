/* 
 * File:   led_blink_pattern.h
 * Author: user
 *
 * Created on March 12, 2022, 5:20 PM
 */

#ifndef LED_BLINK_PATTERN_H
#define	LED_BLINK_PATTERN_H


void ledBlinkpattern (uint8_t num_blinks, uint8_t led_color_rgb, uint16_t blink_on_time_ms, uint16_t blink_off_time_ms, uint16_t starting_blank_time_ms, uint16_t ending_blank_time_ms, uint16_t pwm_fade_out_slope);
void resetLEDBlinkPattern (void);






























#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* LED_BLINK_PATTERN_H */

