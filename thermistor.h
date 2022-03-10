/* 
 * File:   thermistor.h
 * Author: user
 *
 * Created on March 5, 2022, 3:28 PM
 */


#ifndef THERMISTOR_H
#define	THERMISTOR_H

#include <stdint.h>
#include "main.h"

enum {
    voltage = 0,
    temp = 1,
};

/* First value in array is voltage at a given temperature, in 10mV steps. Ex. value of 24 = 240mV
 Second value is temperature in Celsius
 Data was computed in Google Sheets with the following layout and formulas:
 * Column A is temperature in Celsius in 1 degree steps from 0-99C
 * Column B is resistance of thermistor at temp with formula =$L$2/EXP(($L$3*((A2+273.15)-($L$1+273.15)))/((A2+273.15)*($L$1+273.15)))
 * Column C is voltage output (mV) of the resistor + thermistor voltage divider with formula =1000*$L$5*B2/(B2+$L$6)
 * Voltage values in mV were then divided by 10 and rounded to nearest whole number. Duplicate voltage values were removed, retaining highest temp duplicate.
 * Lines containing voltage values greater than 255 were discarded.
 * Variables were:
 * Cell L1 = 25 = Thermistor rating temp in C
 * Cell L2 = 10000 = Thermistor rated resistance at rating temp in Ohms
 * Cell L3 = 3500 = beta of thermistor
 * Cell L5 = 3.3 = Vin for resistor divider calc
 * Cell L6 = 23700 = Thermistor series resistor / voltage divider resistance in Ohms
 */

//You could probably save 160 bytes of flash by removing the celsius values and using the array index as the temperature. Just have to add handling for when two temps would have same voltage.
#define SV09_LUT_SIZE_DEF 93
uint8_t const SV09_thermistor_LUT[SV09_LUT_SIZE_DEF][2] = {
    {45,99},
    {46,98},
    {47,97},
    {48,96},
    {49,95},
    {50,94},
    {51,93},
    {52,92},
    {53,91},
    {54,90},
    {56,89},
    {57,88},
    {58,87},
    {59,86},
    {61,85},
    {62,84},
    {64,83},
    {65,82},
    {66,81},
    {68,80},
    {69,79},
    {71,78},
    {73,77},
    {74,76},
    {76,75},
    {78,74},
    {79,73},
    {81,72},
    {83,71},
    {85,70},
    {87,69},
    {89,68},
    {91,67},
    {93,66},
    {95,65},
    {97,64},
    {99,63},
    {101,62},
    {103,61},
    {105,60},
    {108,59},
    {110,58},
    {112,57},
    {115,56},
    {117,55},
    {120,54},
    {122,53},
    {125,52},
    {127,51},
    {130,50},
    {133,49},
    {135,48},
    {138,47},
    {141,46},
    {144,45},
    {146,44},
    {149,43},
    {152,42},
    {155,41},
    {158,40},
    {161,39},
    {164,38},
    {167,37},
    {170,36},
    {173,35},
    {176,34},
    {179,33},
    {182,32},
    {185,31},
    {188,30},
    {191,29},
    {194,28},
    {198,27},
    {201,26},
    {204,25},
    {207,24},
    {210,23},
    {213,22},
    {216,21},
    {219,20},
    {222,19},
    {225,18},
    {228,17},
    {231,16},
    {234,15},
    {236,14},
    {239,13},
    {242,12},
    {245,11},
    {248,10},
    {250,9},
    {253,8},
    {255,7}
};

#define SV11_LUT_SIZE_DEF 84
uint8_t const SV11_thermistor_LUT[SV11_LUT_SIZE_DEF][2] = {
    {13,99},
    {14,97},
    {15,94},
    {16,91},
    {17,89},
    {18,86},
    {19,84},
    {20,82},
    {21,80},
    {22,79},
    {23,77},
    {24,75},
    {25,74},
    {26,72},
    {27,71},
    {28,69},
    {29,68},
    {30,67},
    {31,66},
    {32,64},
    {33,63},
    {34,62},
    {35,61},
    {36,60},
    {37,59},
    {38,58},
    {39,57},
    {40,56},
    {42,55},
    {43,54},
    {44,53},
    {45,52},
    {47,51},
    {48,50},
    {49,49},
    {51,48},
    {52,47},
    {54,46},
    {55,45},
    {57,44},
    {59,43},
    {60,42},
    {62,41},
    {64,40},
    {66,39},
    {68,38},
    {70,37},
    {72,36},
    {74,35},
    {76,34},
    {78,33},
    {80,32},
    {83,31},
    {85,30},
    {88,29},
    {90,28},
    {93,27},
    {95,26},
    {98,25},
    {101,24},
    {103,23},
    {106,22},
    {109,21},
    {112,20},
    {115,19},
    {118,18},
    {122,17},
    {125,16},
    {128,15},
    {131,14},
    {135,13},
    {138,12},
    {142,11},
    {145,10},
    {149,9},
    {152,8},
    {156,7},
    {160,6},
    {163,5},
    {167,4},
    {171,3},
    {175,2},
    {179,1},
    {182,0}
};

const modelnum_t LUT_SIZE[NUM_OF_MODELS] = {       //This works because the indexes in this array align with the enum modelnum_t from main.h
    SV09_LUT_SIZE_DEF,
    SV11_LUT_SIZE_DEF,
};

//I think this is a two element array of pointers, and each pointer itself points to a two element array.
//The target two element array can then be incremented to parse through the LUT elements.
uint8_t const (*ThermistorLUT[2])[2] = {SV09_thermistor_LUT, SV11_thermistor_LUT};

uint8_t getThermistorTemp (modelnum_t modelnum);














#endif	/* THERMISTOR_H */

