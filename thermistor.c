#include <xc8debug.h>

#include "thermistor.h"
#include "main.h"
#include "config.h"



uint8_t getThermistorTemp (modelnum_t modelnum){    //Binary search algorithm with minor tweaks and rounding
    
    uint16_t pic_thermistor = readADCmV(ADC_THERMISTOR);
    
    uint8_t iteration = 1;
    int16_t increment = LUT_SIZE[modelnum]/(1 << iteration);    //Making this larger and signed because I'm concerned about going out of bounds and being unable to check
    uint8_t index = (uint8_t) increment;   //start at midpoint of array
    
    
    
    //Loop until we find the index where the voltage at i is less than or equal to our read value, and the voltage at i+1 is greater than our read value
    //Meaning we found the two index values our read value is between
    while ( !(ThermistorLUT[modelnum][index][voltage] * 10 <= pic_thermistor) || !(ThermistorLUT[modelnum][index+1][voltage] * 10 > pic_thermistor)  ){
        iteration++;
        if (increment >= 2) increment = LUT_SIZE[modelnum]/(1 << iteration); //Keep halving the increment but make sure increment doesn't go to zero. Using bit shift to calculate power of two, diving LUT_SIZE by bit shift so we don't get successive rounding errors by just dividing the previous increment by two
        if (increment < 1) increment = 1;   //Make sure increment can't go below one
        if ( index + increment > LUT_SIZE[modelnum] - 1 || index - increment < 0){ //if the next increment step would go out of bounds, just break. Subtract one because arrays are zero indexed
            break;
        }
        
        if (ThermistorLUT[modelnum][index][voltage] * 10 < pic_thermistor){
            //Index num is too low
            index = index + (uint8_t) increment;
        }
        else if (ThermistorLUT[modelnum][index][voltage] * 10 > pic_thermistor){
            //Index is too high
            index = index - (uint8_t) increment;
        }
    }
    
    if (index == LUT_SIZE[modelnum] - 1){   //If the resulting index is the max value, reduce it by one so our distance to i calcs work and i+1 doesn't go out of bounds. Arrays are zero indexed.
        index = index - 1;
    }
    
    int16_t dist_to_i = abs((int16_t)ThermistorLUT[modelnum][index][voltage]*10 - (int16_t)pic_thermistor);
    int16_t dist_to_i_plus_one = abs((int16_t)ThermistorLUT[modelnum][index+1][voltage]*10 - (int16_t)pic_thermistor);
    
    if (dist_to_i < dist_to_i_plus_one) {               //If pic_thermistor is closer to i, return i
        return ThermistorLUT[modelnum][index][temp];
    }
    else if (dist_to_i > dist_to_i_plus_one){           //If pic thermistor is closer to i+1, return i+1
        return ThermistorLUT[modelnum][index+1][temp];
    }
    else if (dist_to_i == dist_to_i_plus_one){          //If they are equidistant, round the temperature up (use the lower index))
        return ThermistorLUT[modelnum][index][temp];
    }
    else {
        __debug_break();    //panic
        return ThermistorLUT[modelnum][index][temp]; 
    }
    
    
}
