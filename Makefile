# Makefile - Part of CSchem, (C) Daniel Wagenaar 2021–2025

# This Makefile acts as a frontend to the CMake build process
# On Windows, "make" may not be available, and you will need
# to run the "cmake" calls in "prep-release" and "release"
# manually.

######################################################################
ALL: release

# Release build
prep-release:
	+cmake -S . -B build -DCMAKE_BUILD_TYPE=Release  

release: prep-release
	+cmake --build build --config Release

# Debug build
prep-debug:
	+cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug 

debug: prep-debug
	+cmake --build build-debug --config Debug

# Debian/Ubuntu/etc packaging
deb:	release
	(cd build; cpack )

# Mac OS packaging
dmg:	release
	make -C build dmg

# Source tarball packaging
tar:;	git archive -o ../cschem.tar.gz --prefix=cschem/ HEAD

# Cleaning
clean:; rm -rf build build-debug

######################################################################
.PHONY: ALL release prep-release debug prep-debug clean tar deb dmg

