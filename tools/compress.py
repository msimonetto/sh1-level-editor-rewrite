import os
import struct
from itertools import chain

SECTOR_SIZE = 2352
DATA_OFFSET = 24
DATA_SIZE = 2048

DIRS_NTSC = [
    "1ST", "ANIM", "BG", "CHARA", "ITEM", "MISC", "SND", "TEST", 
    "TIM", "VIN", "XA"
]

FILE_TYPES = [
    "TIM", "VAB", "BIN", "DMS", "ANM", "PLM", "IPD", "ILM", 
    "TMD", "DAT", "KDT", "CMP", "TXT", "UU1", "UU2", ""
]

TOC_OFFSET = 0xB91C
FILE_COUNT = 2074

def compress_assets(bin_path, out_dir):
    os.makedirs(out_dir, exist_ok=True)

if __name == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description="Compress assets from modified SH1 BIN dump.")
    parser.add_argument("bin_path", help="Root path to the extracted files (e.g., D:/assets/BG/... would mean D:/assets/ is directory of interest).")
    parser.add_argument("out_dir", help="Output directory for compressed BIN file.")
    args = parser.parse_args()
    
    extract_assets(args.bin_path, args.out_dir)