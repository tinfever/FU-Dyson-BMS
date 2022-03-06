#include <xc8debug.h>

#include "thermistor.h"
#include "main.h"
#include "config.h"



uint8_t getThermistorTemp (modelnum_t modelnum){
    
    uint16_t pic_thermistor = readADCmV(ADC_THERMISTOR);
    
    modelnum = SV11;
    
    uint8_t increment = LUT_SIZE[modelnum]/2;
    uint8_t index = increment;   //start at midpoint of array
    
    
    //Until we find the index where the voltage at i is less than our read value and the voltage at i+1 is greater than our read value
    //Meaning we found the two index values our read value is between
    while ( !(ThermistorLUT[modelnum][index][voltage] * 10 <= pic_thermistor) || !(ThermistorLUT[modelnum][index+1][voltage] * 10 > pic_thermistor)  ){    //&& !( (ThermistorLUT[modelnum][index+1][voltage] * 10) > pic_thermistor)
        if (increment >= 2) increment = increment/2; //Keep halving the increment but make sure increment doesn't go to zero
        
        if ( index + increment > LUT_SIZE[modelnum] - 1 || index - increment <= 0){ //if the next increment step would go out of bounds, just break. Subtract one because arrays are zero indexed
            break;
        }
        
        if (ThermistorLUT[modelnum][index][voltage] * 10 < pic_thermistor){
            //Index num is too low
            index = index + increment;
        }
        else if (ThermistorLUT[modelnum][index][voltage] * 10 > pic_thermistor){
            //Index is too high
            index = index - increment;
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
