#!/usr/bin/python3

import sys
import tempfile
import os

svgin = sys.argv[1]
base = ".".join(svgin.split(".")[:-1])
base = base.split("/")[-1]
tmpdir = tempfile.TemporaryDirectory()
tmppath = tmpdir.name
resos = [16, 32, 48, 64, 128, 256, 512, 1024]
macresos = resos
macresos2 = [16, 32, 64, 128, 256, 512]
winresos = [16, 32, 64, 128]
linuxresos = [48, 128, 256]

# various sizes
for res in resos:
    if os.system(f"inkscape --export-width {res} --export-height {res} --export-filename {tmppath}/{res}.png {svgin}"):
        raise RuntimeError("inkscape failed")

# build mac icons
os.system(f"mkdir {tmppath}/{base}.iconset")
for res in macresos:
    os.system(f"cp {tmppath}/{res}.png {tmppath}/{base}.iconset/icon_{res}x{res}.png")
for res in macresos2:
    os.system(f"cp {tmppath}/{res*2}.png {tmppath}/{base}.iconset/icon_{res}x{res}@2x.png")
os.system(f"icnsutil -c icns -o generated/{base}.icns {tmppath}/{base}.iconset")

# build win icons
pngs = [f"{tmppath}/{res}.png" for res in winresos]
os.system(f"icotool -c -o generated/{base}.ico " + " ".join(pngs))

# copy linux icons
for res in linuxresos:
    os.system(f"cp {tmppath}/{res}.png generated/{base}_{res}x{res}.png")
