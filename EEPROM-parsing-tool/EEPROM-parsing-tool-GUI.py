import csv, json
from math import comb
from pathlib import Path
from textwrap import wrap
import tkinter as tk
from tkinter import filedialog, messagebox

# Function to parse the file and process data
def process_file(filepath):
    try:
        temp_list = []
        combined_list = []

        with open(filepath) as tsv:
            for line in tsv:
                if line.split():    # Ensure line isn't empty
                    temp_list.append(wrap(line, 2))

        if all(line[0] == format(i, 'X').ljust(2, '0') for i, line in enumerate(temp_list)):
            for line in temp_list:
                line.pop(0)

        for line in temp_list:
            combined_list += line

        falsecount = 0
        for item in combined_list[1::2]:
            if bool(int(item, 16)) == False:
                falsecount += 1

        if falsecount == len(combined_list[1::2]):
            debug_window.insert(tk.END, "All odd values are zero.\n")
            for i, item in enumerate(combined_list[1::2]):
                combined_list.pop(i + 1)

        firmware_version_hex = ''.join(combined_list[:24])
        firmware_version = bytearray.fromhex(firmware_version_hex).decode()
        output_data = {"Firmware": firmware_version}

        total_runtime_hex = ''.join(combined_list[28:32])
        total_runtime_seconds = round(int(total_runtime_hex, 16) * .032, 3)
        output_data["Total_Runtime_Seconds"] = total_runtime_seconds

        combined_list = combined_list[32:]
        fault_list = []

        for i in range(len(combined_list) // 6):
            timestamp_ticks = ''.join(combined_list[2:6])
            timestamp_seconds = round(int(timestamp_ticks, 16) * .032, 3)

            error_code_hex = ''.join(combined_list[:2]),
            error_code_hex = error_code_hex[0]
            error_code_binary = str(bin(int(error_code_hex, 16)))[2:].zfill(16)
            error_bits = list(error_code_binary)

            error_meaning = []
            if error_bits[0] == "1":
                error_meaning.append("ISL_INT_OVERTEMP_FLAG")
            if error_bits[1] == "1":
                error_meaning.append("ISL_EXT_OVERTEMP_FLAG")
            # Add more flags as per the original code

            detect_mode = (''.join(error_bits[14:])),
            detect_mode = int(detect_mode[0], 2)
            detect_mode_text = ["None", "Trigger", "Charger", ""][detect_mode]

            error_dict = {
                "index": i,
                "error_meaning": error_meaning,
                "detect_mode": detect_mode_text,
                "timestamp": timestamp_seconds
            }
            combined_list = combined_list[6:]
            if timestamp_ticks != "FFFFFFFF":
                fault_list.append(error_dict)

        output_data["Faults"] = fault_list
        debug_window.insert(tk.END, json.dumps(output_data, indent=4) + "\n")

    except Exception as e:
        debug_window.insert(tk.END, f"Error: {e}\n")

# Function to select a file
def select_file():
    file_path = filedialog.askopenfilename(title="Select a File", filetypes=[("TXT", "*.txt*"),("EEP files", "*.eep"), ("All files", "*.*")])
    if file_path:
        debug_window.insert(tk.END, f"Selected file: {file_path}\n")
        process_file(file_path)

# Create the main window
root = tk.Tk()
root.title("EEPROM-parsing-tool - GUI By Critec")

# Create a button to select the file
select_file_button = tk.Button(root, text="Select File", command=select_file)
select_file_button.pack(pady=10)

# Create a debug window
debug_window = tk.Text(root, wrap=tk.WORD, height=20, width=80)
debug_window.pack(pady=10)

# Run the GUI loop
root.mainloop()
