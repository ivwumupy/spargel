"""Generate raw DEFLATE compressed stream data.
"""

import argparse
import zlib

parser = argparse.ArgumentParser()
parser.add_argument("output")
parser.add_argument("level", type=int) # level: 0-9, no compression to best compression
args = parser.parse_args()

uncompressed_data = b"This is some data to be compressed using raw DEFLATE."
compressed = zlib.compress(uncompressed_data, args.level, wbits=-15)

with open(args.output, "wb") as f:
    f.write(compressed)
