import csv, json
import argparse
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("file_path", type=Path)

p = parser.parse_args()

if p.file_path.exists():
    filepath = p.file_path
else:
    print("Error: file not found")
    programPause = input('Press <ENTER> to quit')
    exit()


combined_list = []

with open(filepath) as tsv:
    for line in csv.reader(tsv, dialect="excel-tab"):
        line.pop()
        combined_list = combined_list + line

firmware_version_hex = ''.join(combined_list[:24]) 
firmware_version = bytearray.fromhex(firmware_version_hex).decode()
output_data = {}
output_data["Firmware"] = firmware_version

total_runtime_hex = ''.join(combined_list[28:32]) 
total_runtime_seconds = round(int(total_runtime_hex, 16)* .032, 3)  #32ms per tick
output_data["Total_Runtime_Seconds"] = total_runtime_seconds

combined_list = combined_list[32:]

fault_list = []
for i in range(len(combined_list)//6):
    timestamp_ticks = ''.join(combined_list[2:6]) 
    timestamp_seconds = round(int(timestamp_ticks, 16)* .032, 3)  #32ms per tick

    error_code_hex = ''.join(combined_list[:2]),
    error_code_hex = error_code_hex[0]
    error_code_binary = str(bin(int(error_code_hex, 16)))[2:].zfill(16)
    error_bits = list(error_code_binary)

    error_meaning = []
    if error_bits[0] == "1":
        error_meaning.append("ISL_INT_OVERTEMP_FLAG")
    if error_bits[1] == "1":
        error_meaning.append("ISL_EXT_OVERTEMP_FLAG")
    if error_bits[2] == "1":
        error_meaning.append("ISL_INT_OVERTEMP_PICREAD")
    if error_bits[3] == "1":
        error_meaning.append("THERMISTOR_OVERTEMP_PICREAD")
    if error_bits[4] == "1":
        error_meaning.append("UNDERTEMP_FLAG")
    if error_bits[5] == "1":
        error_meaning.append("CHARGE_OC_FLAG")
    if error_bits[6] == "1":
        error_meaning.append("DISCHARGE_OC_FLAG")
    if error_bits[7] == "1":
        error_meaning.append("DISCHARGE_SC_FLAG")

    if error_bits[8] == "1":
        error_meaning.append("DISCHARGE_OC_SHUNT_PICREAD")
    if error_bits[9] == "1":
        error_meaning.append("CHARGE_ISL_INT_OVERTEMP_PICREAD")
    if error_bits[10] == "1":
        error_meaning.append("CHARGE_THERMISTOR_OVERTEMP_PICREAD")
    if error_bits[11] == "1":
        error_meaning.append("TEMP_HYSTERESIS")
    if error_bits[12] == "1":
        error_meaning.append("ERROR_TIMEOUT_WAIT")
    if error_bits[13] == "1":
        error_meaning.append("LED_BLINK_CODE_MIN_PRESENTATIONS")

    detect_mode = (''.join(error_bits[14:])),
    detect_mode = int(detect_mode[0], 2)
    if detect_mode == 0:
        detect_mode_text = "None"
    elif detect_mode == 1:
        detect_mode_text = "Trigger"
    elif detect_mode == 2:
        detect_mode_text = "Charger"
    else:
        detect_mode_text = ""


    error_dict = {
        "index": i,
        #"error_code": error_code_hex,
        "error_meaning": error_meaning,
        "detect_mode": detect_mode_text,
        "timestamp": timestamp_seconds
    }
    combined_list = combined_list[6:]
    if timestamp_ticks != "FFFFFFFF":
        fault_list.append(error_dict)


output_data["Faults"] = fault_list

print(json.dumps(output_data, indent=4))

programPause = input('Press <ENTER> to continue')