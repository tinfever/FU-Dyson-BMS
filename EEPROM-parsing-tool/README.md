EEPROM-parsing-tool.py is a Python script that will convert the EEPROM logged data from the FU-Dyson-BMS firmware into something human readable.

usage: EEPROM-parsing-tool.py file_path

example:
>EEPROM-parsing-tool.py example-eeprom-dump.txt
{
    "Firmware": "Tinfever FU_Dyson_BMS V1",
    "Total_Runtime_Seconds": 388.096,
    "Faults": [
        {
            "index": 0,
            "error_meaning": [
                "DISCHARGE_OC_SHUNT_PICREAD"
            ],
            "detect_mode": "Trigger",
            "timestamp": 53.728
        },
        {
            "index": 1,
            "error_meaning": [
                "CHARGE_OC_FLAG"
            ],
            "detect_mode": "Charger",
            "timestamp": 64.608
        },
...
You can use either the PICKit 3 Standalone Programmer App, or you can use MPLABX to read the EEPROM data.


PICKit 3 Standalone Programmer App
-----------------------------------

Short Instructions:
Read the device using the PICKit 3 standalone app. In the EEPROM data window make sure the data is shown as "Hex Only". Right-click anywhere in the EEPROM data and click Select All, then right-click and Copy. Paste the data in to a .txt file. Run EEPROM-parsing-tool.py with the path to the saved data as the single argument.

Detailed Instructions:
1) Connect PICKit to computer
2) Connect PITKit to BMS board
3) Briefly press button on battery to wake up BMS board (you may also need to place a magnet over the reed switch if using a V7 (SV11) battery). The LED will light up when board is awake.
4) Open the Standalone Programmer App (Probably shown as PICkit 3 v3.01 in the start-menu)
5) The PIC should be automatically detected. The program should show Device: PIC16LF1847. If it doesn't, check your connections, then select Device Family > Midrange > 1.8V Min. Then select Tools > Check Communication
6) Click the Read button
7) Under the EEPROM Data section, make sure the drop down is set to "Hex Only".
8) Right-click anywhere in the EEPROM hex data at the bottom. Click Select All. Right-click again and click copy.
9) Open Notepad and paste the data in to a blank file. Save the file in the same directory as this script.
10) Run this script by running EEPROM-parsing-tool.py your-exported-file-name-here



MPLABX
--------------
Short instructions :
Read the EEPROM data using MPLAB X. View the EE Data Memory window, right click on the data and do "Export Table". Verify it will export address range 0x00 to 0xFF and save the data somewhere. Run EEPROM-parsing-tool.py with the path to the saved data as the single argument.

Detailed Instructions:
1) Connect PICKit to computer
2) Open MPLAB X IDE
3) Confirm PICKit is configured to not power the target device
4) Connect PITKit to BMS board
5) Briefly press button on battery to wake up BMS board (you may also need to place a magnet over the reed switch if using a V7 (SV11) battery). The LED will light up when board is awake.
6) Click the toolbar icon for "Read Device Memory" in MPLAB X (The icon looks like an up arrow. It is near the "Play" icon)
7) Navigate to Window > Target Memory Views > EE Data Memory
8) Right click anywhere in the hex data and select "Export table"
9) Make sure it will export address ranged 0x00 to 0xFF. Do not select "Single Column Ouput". Save it in the same directory as this script.
10) Run this script by running EEPROM-parsing-tool.py your-exported-file-name-here


