#!/usr/bin/python3

import os
import shutil

#%% EXTERNAL PATHS
qt_path = "c:/Qt/5.12.10/msvc2017_64"
qbin_path = qt_path + "/bin"
msvc_path = r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC"

#%% INTERNAL PATHS
root = "C:/Users/Wagenaar/Documents/Progs/cschem"
cschem_build = root + "/build-cschem-Desktop_Qt_5_12_10_MSVC2017_64bit-Release/release"
cpcb_build = root + "/build-cpcb-Desktop_Qt_5_12_10_MSVC2017_64bit-Release/release"
release_path = root + "/release-x64"
#%%
envpath = os.environ['PATH'].split(";")
usepath = [ p for p in envpath if "anaconda" not in p.lower()]
os.environ['PATH'] = ';'.join(usepath)

if not os.path.exists(cschem_build + "/cschem.exe"):
    raise Exception("cschem executable not found")
if not os.path.exists(cpcb_build + "/cpcb.exe"):
    raise Exception("cpcb executable not found")
#%%
if os.path.exists(release_path):
    shutil.rmtree(release_path)
os.mkdir(release_path)
#%%
os.environ['VCINSTALLDIR'] = msvc_path
#%%
os.system(f"{qbin_path}/windeployqt --release --dir {release_path} {cpcb_build}/cpcb.exe")
os.system(f"{qbin_path}/windeployqt --release --dir {release_path} {cschem_build}/cschem.exe")
#%%
shutil.copy(f"{cpcb_build}/cpcb.exe", release_path)
shutil.copy(f"{cschem_build}/cschem.exe", release_path)
#%%

print("Now run 'tools/cschem-x64.iss' using Inno Setup.")
