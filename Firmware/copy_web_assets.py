import os
import shutil
from pathlib import Path

Import("env")

# --- CONFIGURATION ---
NUXT_OUTPUT_DIR = Path("extras/WebPortal/.output/public")
DATA_DIR = Path("data/web")

def copy_web_assets(*args, **kwargs):
    print("\n--- Removing old files ---")
    # Remove old web assets in /data/web
    if DATA_DIR.exists():
        print(f"Cleaning the folder {DATA_DIR}...")
        shutil.rmtree(DATA_DIR)
        print("Old web assets removed.")
    else:
        print("No old web assets found.")

    print("\n--- Copying web optimised files (GZIP) ---")
    
    # 1. Check if the Nuxt build exists
    if not NUXT_OUTPUT_DIR.exists():
        print(f"ERROR: The folder {NUXT_OUTPUT_DIR} does not exist.")
        print("-> Run 'npm run generate' in your WebPortal folder first!")
        return

    # 2. Clean the existing data folder (to avoid old ghost files)
    if DATA_DIR.exists():
        print(f"Cleaning the folder {DATA_DIR}...")
        shutil.rmtree(DATA_DIR)
    
    DATA_DIR.mkdir(parents=True, exist_ok=True)

    # 3. Walk and copy intelligently
    total_size = 0
    file_count = 0

    for root, dirs, files in os.walk(NUXT_OUTPUT_DIR):
        # Create the subfolder structure in /data
        rel_path = os.path.relpath(root, NUXT_OUTPUT_DIR)
        dest_folder = DATA_DIR / rel_path
        dest_folder.mkdir(exist_ok=True)

        for file in files:
            src_file = Path(root) / file
            
            # INTELLIGENT FILTERING
            ignore_conditions = [
                file.endswith(".br"),
                file.endswith(".map"),
                file.startswith(".") or file == "Thumbs.db",
                "meta" in rel_path.split(os.sep)
            ]

            if any(ignore_conditions):
                print(f"  -> Ignored : {file}")
                continue
            
            # D. Copy the file (either it's a .gz or a native asset like .png/.woff2)
            dest_file = dest_folder / file
            shutil.copy2(src_file, dest_file)
            
            total_size += dest_file.stat().st_size
            file_count += 1
            # print(f"   ✅ Copied : {file}")

    # Convert to MB for display
    size_mb = total_size / (1024 * 1024)
    print(f"Done! {file_count} files copied to '/data'.")
    print(f"Total size: {size_mb:.2f} MB")
    print("--------------------------------------------------\n")

copy_web_assets() # force copy at start because dumb platformio executes the script after building the littlefs image

# Run the script before creating the LittleFS image
# env.AddPreAction("buildfs", copy_web_assets)
# env.AddPreAction("uploadfs", copy_web_assets)
# env.AddPreAction(os.path.join("$BUILD_DIR", "littlefs.bin"), copy_web_assets)