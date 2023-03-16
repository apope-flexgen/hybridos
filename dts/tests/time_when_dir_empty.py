import os, os.path
import argparse
import time
from datetime import datetime, timezone

parser = argparse.ArgumentParser(description='Monitors a directory and outputs a timestamp to a file when the directory is empty, then stops. If you want to have this run in the background even when you leave the shell, run it with nohup.')
parser.add_argument('--dir', type=str, help='The directory to monitor.')
parser.add_argument('--out', type=str, help='The file to which we output.')
args = parser.parse_args()

dir = args.dir
out = args.out

empty = False
while not empty:
    time.sleep(1)
    num_files = len([name for name in os.listdir(dir) if os.path.isfile(os.path.join(dir, name))])
    empty = (num_files == 0)

output = datetime.now(timezone.utc).strftime('%Y:%m:%d %H:%M:%S %Z %z') + '\n'

f = open(out, "a")
f.write(output)
f.close()