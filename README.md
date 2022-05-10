# CSchem and CPCB - Electronic circuit design and PCB layout

## Introduction

CSchem and CPCB are a pair of programs for electronic circuit design and
PCB layout. Key features include:

* CSchem makes it easy to draw publication-quality designs.

* CSchem allows you to focus squarely on design principles and worry
  about implementation details later.

* CSchem can export designs as SVG files for direct import into
  Inkscape, Corel Draw, etc.

* CPCB allows linking a schematic to a PCB layout and offers
  highlighting of incomplete or wrongly connected nets.

* CPCB can export layouts as Gerber files for direct upload to
  fabricators such as JLCPCB and many others.

* Both programs have simple and intuitive user interfaces with fast
  learning curves for nonexperts.

CSchem and CPCB are free and open source and can be compiled on Windows, Mac, and
Linux. Please visit their [web page](http://www.danielwagenaar.net/cschem)
for download details.

Development continues, and your suggestions, bug reports, and
contributions are welcome.

## Installation

Installation packages are currently available for Ubuntu
Linux and Windows 10. An installation package for Mac OS X is not hard to make, so let me know if you want it.

## Installation from source on Linux

* Make sure you have the required dependencies. Most importantly, you
  should have the complete Qt development system installed, version
  5.6.1 or above. Packages vary widely across distributions.

  On Ubuntu, you would do:

        sudo apt install git qtmultimedia5-dev libqt5svg5-dev asciidoc

  On OpenSUSE, you would do:

        sudo zypper install --no-recommends git libqt5-qtbase-common-devel \
             libqt5-qtmultimedia-devel libqt5-qtsvg-devel \
	     libqt5-qttools-devel asciidoc

  (I would be happy to include your instructions for other
  distributions. Please [drop me a line](mailto:daw@caltech.edu).)

* Clone the code from github:

        git clone https://github.com/wagenadl/cschem.git

* Enter the project directory:

        cd cschem

* Compile the code:

        mkdir build
	cd build
	cmake ..
	cmake --build .

* Test the result:

        ./cschem
        ./cpcb

* Once you are happy, install CSchem to a system location:

        sudo make install

  (This installs the binaries in `/usr/local/bin` and some extra files in
  `/usr/local/share`. The environment variable CMAKE_INSTALL_PREFIX can be used to specify another location.)
  
  Or, more conveniently, create your own .deb and install that:
  
  	cpack .
	sudo dpkg -i cschem_0.2.0-1_amd64.deb

Please let me know if you have trouble. I will gladly try to help.

## Installation from source on Mac OS X

If you have CMake installed on your system, the steps should be exactly the same, except that CPACK will produce a .dmg. (This has not yet been tested. Please let me know how you fare.)

## Installation from source on Windows

If you have CMake installed on your system, the steps should be exactly the same, except that CPACK will produce a .exe. I like to use the git bash shell to interact with CMake. Double click the resulting .exe to install CSchem.
