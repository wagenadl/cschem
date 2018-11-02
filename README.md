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

An installation package is currently available for Ubuntu
Linux. Binary packages for Windows 10 and Mac OS X will be made
availabel soon.

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

        make

* Test the result:

        ./build/cschem/cschem
        ./build/cpcb/cpcb

* Once you are happy, install ELN to a system location:

        sudo make install

  (This installs the binaries in `/usr/local/bin` and some extra files in
  `/usr/local/share`. If you prefer other locations, please edit lines 10
  and 11 of the Makefile.)

Please let me know if you have trouble. I will gladly try to help.

## Installation from source on Mac OS X

Instructions to appear soon.

## Installation from source on Windows

Instructions to appear soon.
