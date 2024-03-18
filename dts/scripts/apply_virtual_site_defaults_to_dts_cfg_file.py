#!/bin/python3
import argparse
import json

from pathlib import Path


def apply_virtual_site_defaults(dts_cfg_file_path: Path):
    with open(dts_cfg_file_path, 'r') as file:
        dts_cfg = json.load(file)
    
    dts_cfg['input_path'] = '/home/hybridos/historian/inbox'
    dts_cfg['influx_address'] = 'influx-container:8086'
    dts_cfg['mongo_address'] = 'mongo-container:27017'
    
    with open(dts_cfg_file_path, 'w') as file:
        json.dump(dts_cfg, file, indent=4)
        
        
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--cfg", help="Path to DTS configuration file.")
    args = parser.parse_args()
    
    dts_cfg_file_path = Path(args.cfg)
    try:
        apply_virtual_site_defaults(dts_cfg_file_path)
    except Exception as e:
        print(f"Error applying Virtual Site defaults to {dts_cfg_file_path}: {e}.")
        exit(1)
    
    print(f"Successfully applied Virtual Site defaults to {dts_cfg_file_path}.")