import re
import subprocess
import os

Import("env")

def get_firmware_version():
    try:
        ret = subprocess.run(["git", "describe", "--tags", "--always", "--dirty"], stdout=subprocess.PIPE, text=True)
        return ret.stdout.strip()
    except:
        return "0.0.0-unknown"

version = get_firmware_version()
match = re.findall(r"\d+\.\d+\.\d+", version)
clean_version = match[0] if match else "-.-.-"

# for devlopment builds, set version to 0.0.0 to always propose an update
if "dev" in version or "dirty" in version:
    clean_version = "0.0.0"

env.Append(CPPDEFINES=[
    ("FIRMWARE_VERSION", f'\\"{clean_version}\\"')
])