import argparse
import json
import pathlib
import struct

parser = argparse.ArgumentParser()
parser.add_argument("bin", type=pathlib.Path)
parser.add_argument("out")
args = parser.parse_args()

data = args.bin.read_bytes()

n = len(data) // 80

events = []

for i in range(n):
    offset = i * 80
    e = data[offset:offset+80]
    name, kind, timestamp = struct.unpack('64sIQ', e)
    x = name.split(b'\x00')[0]
    name = x.decode('utf-8')
    if kind == 0:
        ph = 'B'
    elif kind == 1:
        ph = 'E'
    else:
        raise

    events.append({
        'name': name,
        'ph': ph,
        'ts': timestamp,
        # pid is required; just provide a dummy value
        'pid': 1000,
    })

with open(args.out, "w") as f:
   json.dump(events, f)
