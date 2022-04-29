/*
* FU-Dyson-BMS	-	(unofficial) Firmware Upgrade for Dyson BMS - V6/V7 Vacuums
* Copyright (C) 2022 tinfever
* 
* This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
* 
* The author can be contacted at tinfever6@(insert-everyone's-favorite-google-email-domain).com
* 
* NOTE: As an addendum to the GNU General Public License, any hardware using code or information from this project must also make publicly available complete electrical schematics and a bill of materials for such hardware.
*/

#ifndef LED_BLINK_PATTERN_H
#define	LED_BLINK_PATTERN_H


void ledBlinkpattern (uint8_t num_blinks, uint8_t led_color_rgb, uint16_t blink_on_time_ms, uint16_t blink_off_time_ms, uint16_t starting_blank_time_ms, uint16_t ending_blank_time_ms, int8_t pwm_fade_slope);
void resetLEDBlinkPattern (void);
void Set_LED_RGB(uint8_t RGB_en, uint16_t PWM_val);
bool cellDeltaLEDIndicator (void);
bool cellVoltageLEDIndicator (void);





























#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* LED_BLINK_PATTERN_H */

