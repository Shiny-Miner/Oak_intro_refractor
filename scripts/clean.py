#!/usr/bin/env python3
"""python scripts/clean.py [build]   remove build output (and generated linker/offsets)."""
import os
import shutil
import sys

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
shutil.rmtree(os.path.join(root, 'build'), ignore_errors=True)
for f in ('offsets.ini', 'linker.ld'):
    p = os.path.join(root, f)
    if os.path.exists(p):
        os.remove(p)
print('cleaned')
