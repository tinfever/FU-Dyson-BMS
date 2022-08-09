
  
# FU-Dyson-BMS
### An (Unofficial) Firmware Upgrade for Dyson V6/V7 Vacuum Battery Management System (BMS)
![Github Header Image](https://user-images.githubusercontent.com/46428760/168486653-8b8b696d-0bcb-4679-95c9-0377f26ec008.jpg)

------

**Dyson vacuum batteries are designed to fail.**

Here's why:

1.  Series battery cells in a battery pack inevitably become imbalanced. This is extremely common and why cell balancing was invented.
2.  Dyson uses a very nice ISL94208 battery management IC that includes cell balancing. It only requires 6 resistors that cost $0.00371 each, or 2.2 cents in total for six. [^1]
3.  Dyson did not install these resistors. (They even designed the V6 board, PCB 61462, to support them. They just left them out.)
4.  Rather than letting an unbalanced pack naturally result in lower usable capacity, when the cells go moderately (300mV) out of balance (by design, see step 3) Dyson programmed the battery to stop working...permanently. It will give you the 32 red blinks of death and will not charge or discharge again. It could not be fixed. Until now. [^2]

<img src="https://user-images.githubusercontent.com/46428760/168484825-c1c25766-edcf-4af1-a2fb-f534d9acf39a.png" width="400" />

FU-Dyson-BMS is a replacement firmware for the microcontroller inside Dyson V6/V7 vacuum batteries. By using this firmware, your battery pack will not become unusable if the cells become imbalanced, you will just have reduced battery capacity as usual. It will also allow you to replace the battery cells to repair your battery, rather than be forced to replace it.

Demonstration, disassembly, and programming video:

https://www.youtube.com/watch?v=dwyA5rBjncg

[![FU-Dyson-BMS - (Unofficial) Firmware Upgrade for Dyson V6/V7 Vacuum Battery Management System](https://img.youtube.com/vi/dwyA5rBjncg/0.jpg)](https://www.youtube.com/watch?v=dwyA5rBjncg)

## Revolutionary features:
-   Cell balance LED indicator
-   State of charge LED indicator
-   Robust fault handling and logging
-   Total runtime tracking
-   Can be run in debug mode for near-real-time diagnostics
-   Doesn't brick itself!
-   Doesn't generate e-waste and try to take your money when your cells go out of balance!

## Why you would want this:
-   You want to vacuum your apartment but your cells became slightly out of balance because you left the vacuum off the charger for too long and now your vacuum doesn’t work (ask me how I know)
-   You want to replace a bad cell in your battery pack
-   You want to understand what your battery is doing and why.
-   You don’t like feeling like a cash cow being squeezed for all you’re worth.
    
## Compatible vacuums/batteries:
-   Dyson V7 - Model SV11 - PCB 279857 - Compatible + Tested
<img src="./hardware-info/images/V7 SV11 - PCB 279857.jpg" width="400" />

-   Dyson V6 - Model SV04/SV09 - PCB 61462 - Compatible + Tested
<img src="./hardware-info/images/V6 SV04,SV09 - PCB 61462.jpg" width="400" />

-   Dyson V6 - Model SV04 - PCB 188002 - Compatible + Tested
<img src="./hardware-info/images/V6 SV04 - PCB 188002, Repaired.jpg" width="400" />

Note: the model numbers are kind of weird. There are three different ways to identify/categorize your vacuum:
1.  The advertised version number (V6, V7, etc)
2.  The actual model number printed on the battery (SV04, SV09, SV11)
3.  The part number printed on the battery PCB (61462, 279857, 188002).
    
Some models like SV04 contain different versions of the battery PCB. Many of these PCB versions are extremely similar and I have no idea why Dyson seems to have made at least 5 different versions. I recommend you use the PCB part number for reference if possible, or the model number printed on the battery otherwise. I still use the V6, V7 names in some places since that is what most people are familiar with, and I keep changing my mind as to which identification method is better.

**Probably not compatible:**
(Although it’d be interesting to see PCB photos of them to be sure)
-   V10 vacuums
-   V11 vacuums
-   Anything newer

If you aren’t sure if your battery is compatible, please submit a Github issue with the highest quality photos possible of the battery PCB and provide the advertised model number (V6, V7, etc) and printed model number (SV09, SV11, etc) and I’ll try to tell you if it will work.

## How to install it:

Warning: The firmware flash process is irreversible. It is not possible to restore the factory firmware.

Summary:

0. Be careful. Li-ion batteries are no joke and must be respected. You're working on a live battery pack that can output 100+ Amps if short-circuited.
1.  Disassemble battery pack to access PCB
2.  Make sure all cells are charged above 3V and that the pack LEDs do *something* when you press the button (with magnet on reed switch if using V7). This confirms the 3.3V rail is regulating and the PIC is awake/working.
3.  Remove conformal coating over programming connection points (if applicable)
4.  Connect PICkit to computer and, if you using a PICkit 3 or clone, install the [PICkit 3 Programmer App and Scripting Tool v3.10](https://ww1.microchip.com/downloads/en/DeviceDoc/PICkit3%20Programmer%20Application%20v3.10.zip). (https://www.microchip.com/en-us/tools-resources/archives/mplab-ecosystem)
5.  Connect PICkit to BMS board as shown below:
<img src="./hardware-info/images/PICkit Wiring Diagram_crop.jpg" width="720" />
6.  Wake up battery pack by pressing button and placing magnet on reed switch (if using V7 vacuum)
7.  While maintaining tension on wires to BMS board, make sure PICkit can see the PIC16LF1847 microcontroller, then import and write the hex file from the latest GitHub release.  
<br/><br/>
For more details, see video linked at the top (https://www.youtube.com/watch?v=dwyA5rBjncg).  
<br/><br/>

<sub><sup>Disclaimer: Lithium-ion batteries can be dangerous and must be respected. Proper cell balancing may reduce this danger which is why only trained professionals who implement cell balancing according to the manufacturer recommended best practices should work on them...wait...well that doesn't include Dyson either so I guess we're on our own. According to the internet, they can spontaneously catch fire, burn your house down, drain your retirement fund, and run away with your wife. Consider yourself warned, and please don't sue me if something goes wrong because I assume no liability and provide no warranty. See section 15 and 16 of the COPYING file for more details.

## Miscellaneous Thoughts on Repairing a Battery Pack 
![Battery Cell Connections and Measurement Points](https://user-images.githubusercontent.com/46428760/168488794-7eb77a0e-5629-472b-b02a-e23a85353311.jpg)
If you left your battery in storage for a long time, you may have found it no longer turns on at all and won’t charge either. This is because the battery cells have self-discharged so low that the ISL94208 won’t even turn on, which means the microcontroller won’t turn on either.

If you connect a constant current power supply directly to the terminals of the battery pack bypassing the BMS board, you can slowly recharge the cells until they are back within a normal voltage range (above 3V). I've found the [PCBite probes](https://store-usa.arduino.cc/products/4x-sp10-probes-and-test-wires?selectedStore=us) to work well for easily connecting any cell or pack to a bench power supply. Soldering small wires to the nickel strips or jamming on alligator clips somehow would probably work too. I recommend charging at 50-100mA until all cells are over 3V. For safety, you don’t want to charge a battery that’s been depleted too far at the normal charge current (700mA).

After all cells are above 3V, the BMS should power up as usual. If you aren’t getting the 32 red blinks of death, you might not even need to install this firmware (as much as it pains me to admit it). While you have the battery disassembled, I’d recommend making sure all the battery cells are within 100mV of each other, and manual charge the lower cells to get them in that range.

Note: When charging cells that have been over-discharged, you should monitor them carefully to make sure they are taking a charge (the voltage is actually increasing), they aren't getting hot, and the cell voltages are gradually moving in to an acceptable range. Even if some of your cells are extremely out of balance, don't worry about that until you get them all above 3V. Having one cell at 1V and another at 2V might look really bad, but when they are back in range, they might look more like 3.1V and 3.2V.

If your battery isn’t turning on at all, do the following (do not leave unattended while charging):

1.  Disassemble your battery pack.
2.  Measure the voltage of all of the battery cells. You’ll probably find one or many are below 3V.
	-  If your cells are all within 1V of each other and none are negatively charged: Using a bench power supply, charge the entire pack directly across the two large metal terminals that come off cell 1 and cell 6 and connect to the BMS board. This will bypass the BMS and charge the cells directly. Charge at 50-100mA constant current, with a voltage limit of 20V.
	-  If your cells are more than 1V from each other: Use a bench power supply to charge the low cells individually to match the higher cells. Then charger the entire pack directly as mentioned in the previous bullet point.
	-  If any cells are reverse charged, meaning they have a negative voltage where it should normally be positive, you’ll probably need to replace that battery cell. This would involve cutting the nickel strips connected to it, removing it from the battery pack, and spot welding in a new cell. This is beyond the scope of this documentation.
  
  
  
  

## What do the LEDs mean?

  

**While pulling trigger:**
-   Red-Green-Blue flashes - Looks fancy and indicates you’re running the custom firmware
-   Solid Blue - The vacuum is on / Power output is enabled
-   3x Blue flashes - Battery low (Low voltage cutoff reached).
	-   Output disabled until charger connected or pack goes to sleep and forgets
   
**When you release the trigger:**
-   Green flashes - (Rough) Remaining Battery Capacity
	- Indicates (roughly) the remaining battery capacity on a scale of 1-6 flashes, with 6 being completely full and 1 being effectively empty.
	-   1 flash = 3.0V < Min cell < 3.2V
	-   2 flashes = 3.2V < Min cell < 3.4V
	-   3 flashes = 3.4V < Min cell < 3.6V
	-   4 flashes = 3.6V < Min cell < 3.8V
	-   5 flashes = 3.8V < Min cell < 4.0V
	-   6 flashes = 4.0V < Min cell < 4.2V
	-   (Min cell means the voltage of whatever battery cell has the lowest voltage)
	-   (Why 1-6 flashes? Well if 0 flashes was an option, you couldn't tell if the battery meter function was working at all or not)
	- Due to the current draw of the vacuum and the ESR of the cells, 3 flashes could probably be considered almost dead.

**When you connect the charger:**

-   Yellow flashes - Cell Balance Indicator
	-  Indicates how out of balance your battery pack is.
	-   Represents the voltage difference between your highest and lowest voltage cell.
	-   Each flash = 50mV
	-   Example: The highest voltage cell in your pack is 3.95V. The lowest voltage cell is 3.62V. 3.95V - 3.62V = 330mV difference. 330mv / 50mv per flash = 7 flashes (6.6 rounded to 7)
-   Solid blue - Charging is active
-   Solid white - Charging pause/wait
	- The highest voltage cell reached 4.2V so charging was disabled
	-   It will wait for 70 seconds to let the battery cells recover a bit and then resume charging.
-   Solid green - Charging is complete/Idle
	-   Once it takes less than 10 seconds of charging (blue LED) to reach the max cell voltage of 4.2V, charging will be marked as complete
	-   Will sleep after 30 seconds of no activity
    
**When you disconnect the charger:**
-   Yellow flashes - Cell Balance Indicator
	-   (See entry under "When you connect the charger")

**When you hold down the trigger and connect the charger:**

-   White flashes - Firmware version
	-   One white flash = version 1. Four white flashes = version 4, etc
	-   Charging will resume as normal after this is shown.
    

  

**At any time:**
-   Solid green - Battery pack is idle. The output isn't enabled and it isn't charging.
	-   Will sleep after 30 seconds of no activity
-   Red flashes - Fault indicator/Error code
	-   How you should handle errors:  Make note of how many flashes are in your error code, make sure the charger is removed and trigger is released, and then wait 60 seconds for the error code to clear. Then you can try again if you want.
    


## What do the error codes mean?
    
|Number of Red Flashes|Fault Name|Fault Meaning|Default Limit|
|--|--|--|--|
|4|ISL_INT_OVERTEMP_FLAG|ISL94208 asserted flag that it reached the internal over-temperature limit|125C
|5|ISL_EXT_OVERTEMP_FLAG|ISL94208 asserted flag that it measured the external thermistor to be above the over-temperature limit|Temp3V/13 = 3.3V/13 = 254mV = 74C on V7 battery
|6|ISL_INT_OVERTEMP_PICREAD|PIC has read the internal temperature of the ISL94208 to be over the software over-temperature limit|60C
|7|THERMISTOR_OVERTEMP_PICREAD|PIC has read the external thermistor to be over the software over-temperature limit|60C
|8|CHARGE_OC_FLAG|ISL94208 asserted flag that the charging current was over the charge over-current limit|1.4A for 2.5ms (Same as stock firmware behavior. Allows for brief inrush current when wall charger is connected)
|9|DISCHARGE_OC_FLAG| ISL94208 asserted flag that the discharge current was over the discharge over-current limit|50A for 2.5ms (Can’t be set lower)
|10|DISCHARGE_SC_FLAG|ISL94208 asserted flag that the discharge current was over the discharge short-circuit current limit|175A for 190us (Next lowest setting of 100A is insufficient to start vacuum)
|11|DISCHARGE_OC_SHUNT_PICREAD|PIC read the discharge current shunt to be over the software discharge over-current limit|30A (Vacuum uses approx. 3A in normal mode, 17A in Max mode)
|12|CHARGE_ISL_INT_OVERTEMP_PICREAD|PIC has read the ISL94208 internal temp sensor to be over the software over-temperature limit, and the state was charging at time of error|50C
|13|CHARGE_THERMISTOR_OVERTEMP_PICREAD|PIC has read the external thermistor to be over the software over-temperature limit, and the state was charging at time of error|50C
|14|UNDERTEMP_FLAG|Either the thermistor or the ISL94208 temp was measured by the PIC to be below under-temp limit|7C (lowest value included in V7 thermistor LUT in code)
|15|CRITICAL_I2C_ERROR|There was an unrecoverable I2C communication error between the PIC and the ISL94208.|
|16|ISL_BROWN_OUT|ISL94208 has silently reset itself. This usually occurs due to a hard short circuit that isn’t quite large enough to trip the 175A short-circuit limit.|
|20|Unidentified error|This shouldn’t happen|

Error codes will be repeated until:
1) The trigger is released/charger is removed
2) The error reason is no longer present (Example: if you have an over-temperature error, the temperature must have come back within the limits)
3) The error code has been presented at least three times.

However, the pack will go to sleep if it remains in an error state for 60 seconds, regardless of the previous criteria. It will not sleep if the error occurred while the battery was on the charger; in this case the error code will be repeated until the charger is disconnected (so you are always aware of any errors).

  **For more error information, you can dump the EEPROM data and use the EEPROM-parsing-tool to read the exact error codes, timestamp, and trigger/charge state at time of error.** https://github.com/tinfever/FU-Dyson-BMS/tree/main/EEPROM-parsing-tool
  

## How does the firmware work?

<img src="./firmware-info/Firmware State Flow Chart - FINAL.drawio.png"/>
	
**Known Issues:**
- The BMS will go to sleep and put the ISL94208 to sleep after charging is complete. This may create a small but noticeable current draw on the cell connected to VBACK on the ISL94208. This means over the period of months, I think the cell connected to VBACK may slowly go out of balance compared to the other cells. This is not damage in any fashion, but since we can only charge the entire pack in series, any discharge of a specific cell will inevitably cause an imbalance. An imbalance can be fixed by manually charging the imbalanced cell back up to match the others. As a workaround for this issue, I'd recommend not leaving the battery connected to the charger 24/7. Again, no damage will occur, but one of the cells may be discharged slightly and need to be rebalanced. I'll fix this as soon as I can, no ETA though.
- The code is bad. Like really bad. Like, "Oh god, I've created a monster" bad. Like, "I think I may have created something so unnecessarily complicated that I'm not sure I can ever fully understand it" bad. Honestly, the LED codes should probably be broken out in to their own state machine and some serious thought should be put in to whether it's the best idea for each state to handle the transitions to every other state. Also, interrupts should probably have been used...somewhere. However, it works and there are no bugs I'm currently aware of. I'm also afraid to mess with it because there is a good chance I'll break some obscure state transition logic I've either purposely or accidentally patched over with some other piece of logic.
- On one of the BMS boards, there is a circuit that appears to provide the ability to enable the output with a series 33 Ohm resistor. I have no idea what this function could be for and it isn’t on the other BMS boards, so I haven’t implemented it.
- If you connect the charger, the battery becomes fully charged, the pack goes to sleep, and then you remove the charger, the cell balance indicator won't show. This is because the microcontroller is completely turned off until the WKUP signal goes low (by removing the charger) and then high again (by pulling the trigger or attaching the charger).
- Cell balancing is not implemented. I know this is ironic, but because the cell balancing resistors aren't installed and Dyson used 1K resistors for the VCELL# connections, even if you shorted out the connections where the cell balancing resistors would go, which most people aren't going to do (and you'd have to cut some very fine traces on the V7 BMS PCBs), the cell balancing would be extremely slow through the 1K resistors. You'd also have to either add #define setting or figure out some way for the firmware to detect if cell balance resistors are installed and then remember it, because you'd need the pack to stay awake on the charger while balancing if applicable, but go to sleep on the charger otherwise. If the pack thinks it is balancing but there are no balance connections, it would stay awake forever. Also I'm burnt out on this project and have worked on my vacuum enough for one lifetime.
- If you use the battery hard enough for it to get over 50C, and then connect it to the charger, it will immediately trip a charge over-temp error. Since the error will have occurred while it was on the charger, the error will not clear until you remove the battery from the charger, and then wait for the error code to clear and the pack to cool below 50C. Then you can reconnect it to the charger.
- There is no under-voltage charging cutoff. If the ISL94208 and PIC turn on and you connect the charger, it will charge (unless the max cell is over 4.2V of course). In theory this isn't great because it means a low voltage cell, like one at 2.3V, might end up being charged at full current (~700mA).


## FAQ

**Q: Will this work with my vacuum XYZ?**
	
A: If it is a Dyson V6 or V7, probably. Otherwise, probably not. The best way to tell would be to disassemble your battery and see if you have a PCB number that matches one of the tested models. If it matches, it will very likely work. If it doesn't match, submit a Github issue with a high-res photo of the board and I'll try to tell you. If it has a PIC16LF1847 microcontroller with a ISL94208 battery management front-end, there is a good chance it will work and if there is a version like that but that doesn't currently work, I'm open to adding support.

**Q: The batteries aren't designed to fail. They just had to keep costs down.**
	
A: That's not a question. However, if we accept the line of thinking that Dyson truly had to save those 2.2 cents per battery pack, there are other changes they could have made to save a lot more than that. They decided to add the reed switch to the V7 batteries. That probably added a lot more than 2.2 cents. They also added secondary over-charge protection ICs on the V7 batteries. Those probably aren't cheap. I think they also could have replaced the MOSFET they use to allow the over-charge protection ICs to pull-down one of the charge control MOSFETs with a BJT and saved a few cents there. They also could have probably found a different battery management IC without cell balancing that was cheaper. Heck, they might have been able to find a battery management IC that didn't require an additional microcontroller, and saved a whole dollar!

## How you can help
- Install the firmware on your battery and report back how it works, and if you have any issues.
- If your battery isn't compatible, post high-res photos of the PCB here or somewhere on the internet just to promote more freely available technical information.
- Constructive criticism on the code is welcome, although I'm highly unlikely to make any major code changes beyond bug fixes at this point. I am interested in learning how I could improve for future projects though.
- Support [right-to-repair](https://fighttorepair.org/). This project would have been much easier if I didn't have to reverse engineer the entire BMS...twice. Even though anti-repair practices are a separate issue from planned obsolescence, some parts sure feel pretty similar. Also, I asked Dyson for schematics to the BMS so I might be less likely to burn my house down; they offered me a discount on a new battery.

## Other resources
- Full reverse-engineered schematics (with KiCad originals) for BMS boards V6  - Model SV04 - PCB 188002 and V7 - Model SV11 - PCB 279857 are located in the [hardware-info folder](./hardware-info)
- High resolution PCB photos are located in the [hardware-info/images folder](./hardware-info/images)
- Photos with nearly all PCB traces connections overlaid are located in the [hardware-info/images folder](./hardware-info/images). I lovingly call these PCB Spaghetti Wiring Diagrams. You'll see why. If you want to determine how the components on the PCB correlate to the schematic, you can use this. It's not pretty but it works. GIMP original files are also included. I recommend finding the pin number on the PIC or ISL of the net you are looking for, and then looking at these diagrams to see where that net connects to on the actual PCB.
- High-res image and PDF versions of the firmware state flow chart are located in the [firmware-info folder](./firmware-info). Draw.io original files are also included.
- As mentioned earlier, there is a script called [EEPROM-parsing-tool.py](./EEPROM-parsing-tool) that you can use to convert a raw EEPROM dump from this firmware in to something human readable. It will show the firmware version, total battery runtime in seconds (since last firmware flash), and any faults logged along with a timestamp of the fault.
- [EEVBlog Forum Thread for Discussion](https://www.eevblog.com/forum/projects/fu-dyson-bms-an-(unofficial)-firmware-upgrade-for-dyson-v6v7-vacuum-bms/)
	
## Credit
- DavidAlfa from EEVBlog Forum (Created I2C Library)
- dvd4me from EEVBlog Forum (Helped with reverse engineering and provided continued support)
-----
Now, if you’ll excuse me, I’m going to finally vacuum my apartment.

**In memory of BMS boards SV11 #1, SV09 #1, SV04 #1, and SV04 #3 who gave their lives for this project. Their sacrifice will not be in vain.**

[^1]: https://www.digikey.com/en/products/detail/stackpole-electronics-inc/RMCF1206JT100R/1757426
Cost in 5000 qty is $0.00371 each. 100R balance resistor = ~42mA balance discharge current with 176mW power dissipation.

[^2]: This is a slight exaggeration. dvd4me on the EEVblog forums figured out which EEPROM values you can reset in order to un-brick the battery that way. https://www.eevblog.com/forum/reviews/dyson-v7-trigger-cordless-vacuum-teardown-of-battery-pack/msg4028665/#msg4028665
