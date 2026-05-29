import subprocess
import time
import os

# Configuration
# Shutter speeds in MICROSECONDS (1,000,000 = 1 second)
shutter_speeds = [100000, 100001, 100002, 100003, 1000004, 100005, 100006] 
output_folder = "bracketing_test"
gain = 1 # Constant ISO/Gain to ensure only shutter speed changes

if not os.path.exists(output_folder):
    os.makedirs(output_folder)

def take_bracketed_images():
    print(f"Starting capture of {len(shutter_speeds)} images...")
    
    for speed in shutter_speeds:
        filename = f"{output_folder}/flat_field_shutter_{speed}us.jpg"
        
        # Build the libcamera command
        # --shutter: sets fixed exposure time
        # --gain: sets fixed analog gain (prevents auto-ISO from fighting your shutter choice)
        # --immediate: skips the signal-based wait
        cmd = [
            "rpicam-still",
            "-n",                      # No preview window
            "-o", filename,            # Output path
            "--shutter", str(speed),   # Fixed shutter speed
            "--lens-position", "3.15",
            "--gain", str(gain),       # Fixed gain
            "--analoggain", "1",
           # "--awbgains", "2.51,2.8",
            "--immediate"              # Capture quickly
        ]
        
        try:
            print(f"Capturing at {speed}µs...")
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error capturing at {speed}: {e}")
            
    print("Done! Check the folder:", output_folder)

if __name__ == "__main__":
    take_bracketed_images()
