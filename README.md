FU-Dyson-BMS
(Unofficial) Firmware Upgrade for Dyson V6/V7 Vacuum Battery Management System

Dyson vacuum batteries are designed to fail.

Here's why:





"Battery cell balancing is critical for long life of any battery pack. That's why I hand balance every battery cell in my Tesla." - Abraham Lincoln

Ground breaking features:


Disclaimer: Lithium-ion batteries can be dangerous and must be respected. Proper cell balancing may reduce this danger which is why only trained professionals who implement cell balancing according the manufacturer recommended best practices should work on them...wait...well that doesn't include Dyson either so I guess we're on our own. According to the internet, they can spontenously catch fire, burn your house down, drain your retirement fund, and run away with your wife. Consider yourself warned, and please don't sue me if something goes wrong because I assume no liability and provide no warranty. Also, I clearly have more time than money on my hands so suing me wouldn't help much. If you like legaleze, I think there is some under section 15 and 16 of the COPYING file.

Why you would want this:


How to install it:




What do the lights mean?

First of all, red LED flashes are reserved for error codes. The code is indicated by the number of red flashes. It will be repeated at least three times.

tl;dr on error codes: Count how many flashes are in your error code, make sure the charger is removed and trigger is released, and then wait 60 seconds for the error code to clear. Then you can try again if you want.

 The error code will be repeated until: 1) the trigger is released/charger is removed, 2) the error reason is no longer present (Example: if you have an over-temperature error, the temperature must have come back within the limits), and the error code has been presented at least three times. However, the battery will go to sleep if it remains in an error state for 60 seconds, regardless of the previous criteria. It will not sleep if the error occured while the battery was on the charger; in this case the error code will be repeated until the charger is disconnected (so you are always aware of any errors). 

When you pull the trigger:
Red-Green-Blue flashes means you aren't just running the boring Dyson firmware, you're running something better!
(Everytime you pull the trigger to turn on the vacuum, this will happen to indicate that you aren't just running the stock firmware...and it looks cooler.)

Solid Blue - The vacuum is on / Power output is enabled

Three blue flashes - Battery low. The battery cells have reached the minimum cut off voltage of 3V. You should charge it. You will not be able to enable the output until you charge it, or you wait for the battery to go to sleep so it forgets.

When you release the trigger:
Green flashes - Indicates (roughly) the remaining battery capacity on a scale of 1-6 flashes, with 6 being completely full and 1 being effectively empty.
1 flash = 3.0V < Min cell < 3.2V
2 flashes = 3.2V < Min cell < 3.4V
3 flashes = 3.4V < Min cell < 3.6V
4 flashes = 3.6V < Min cell < 3.8V
5 flashes = 3.8V < Min cell < 4.0V
6 flashes = 4.0V < Min cell < 4.2V
(Min cell means the voltage of whatever battery cell has the lowest voltage)
(Why 1-6 flashes? Well if 0 flashes was an option, you couldn't tell if the battery meter function was working at all or not)

Solid green - The battery pack is idle. The output isn't enabled and it isn't charging. It will go to sleep after 30 seconds of no activity.

When you attach the charger:
Yellow flashes - Indicates how out of balance your battery pack is. It represents the voltage difference between your highest and lowest voltage cell. Each flash = 50mV. Example: The highest voltage cell in your pack is 3.95V. The lowest voltage cell is 3.62V. 3.95V - 3.62V = 330mV difference. 330mv / 50mv per flash = 7 flashes (6.6 rounded to 7)
Solid blue - Charging is active
Solid white - Charging wait. The highest voltage cell reached 4.2V so charging was disabled. It will wait for 70 seconds to let the battery cells recover a bit and resume charging.
Solid green - Charging is complete. Once it takes less than 10 seconds of charging (blue LED) to reach the max cell voltage, charging will be marked as complete. The battery will go to sleep after 30 seconds.

When you disconnect the charger: 
Yellow flashes - Indicates how out of balance your battery pack is. (See entry under "When you attach the charger")


When you hold down the trigger and connect the charger:
White flashes - Firmware version. 1 white flash = version 1. 4 white flashes = version 4, etc. Charging will resume as normal after this is shown.