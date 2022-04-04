#!/bin/zsh

cd `dirname $0`/..

######################################################################
# CSCHEM
SUBDIRS="circuit file svg ui"
echo "# Automatically generated by updatesources.sh" > cschem/CMakeLists.txt
echo "" >> cschem/CMakeLists.txt
echo "target_include_directories(cschem PUBLIC cschem)" >> cschem/CMakeLists.txt

find cschem -name \*.h -exec echo "target_sources(cschem PRIVATE" '{}' ")" ';' | sort -df >> cschem/CMakeLists.txt
find cschem -name \*.cpp -exec echo "target_sources(cschem PRIVATE" '{}' ")" ';' | sort -df >> cschem/CMakeLists.txt
find cschem -name \*.ui -exec echo "target_sources(cschem PRIVATE" '{}' ")" ';' | sort -df >> cschem/CMakeLists.txt
find cschem -name \*.qrc -exec echo "target_sources(cschem PRIVATE" '{}' ")" ';' | sort -df >> cschem/CMakeLists.txt

######################################################################
# CPCB
SUBDIRS="data gerber ui"
echo "# Automatically generated by updatesources.sh" > cpcb/CMakeLists.txt
echo "" >> cpcb/CMakeLists.txt
echo "target_include_directories(cpcb PUBLIC cpcb)" >> cpcb/CMakeLists.txt
echo "target_include_directories(cpcb PUBLIC cschem)" >> cpcb/CMakeLists.txt

find cpcb -name \*.h -exec echo "target_sources(cpcb PRIVATE" '{}' ")" ';' | sort -df >> cpcb/CMakeLists.txt
find cpcb -name \*.cpp -exec echo "target_sources(cpcb PRIVATE" '{}' ")" ';' | sort -df >> cpcb/CMakeLists.txt
find cpcb -name \*.ui -exec echo "target_sources(cpcb PRIVATE" '{}' ")" ';' | sort -df >> cpcb/CMakeLists.txt
find cpcb -name \*.qrc -exec echo "target_sources(cpcb PRIVATE" '{}' ")" ';' | sort -df >> cpcb/CMakeLists.txt

FILES="
cschem/circuit/IDFactory.h
cschem/circuit/Schem.h
cschem/circuit/Circuit.h
cschem/circuit/Element.h
cschem/circuit/Connection.h
cschem/circuit/PartNumbering.h
cschem/circuit/SafeMap.h
cschem/circuit/Net.h
cschem/circuit/Textual.h
cschem/file/FileIO.h
cschem/svg/XmlElement.h
cschem/svg/XmlNode.h
cschem/svg/Symbol.h
cschem/svg/SymbolLibrary.h
cschem/circuit/IDFactory.cpp
cschem/circuit/Schem.cpp
cschem/circuit/Circuit.cpp
cschem/circuit/Element.cpp
cschem/circuit/Connection.cpp
cschem/circuit/PartNumbering.cpp
cschem/file/FileIO.cpp
cschem/svg/XmlElement.cpp
cschem/svg/XmlNode.cpp
cschem/svg/Symbol.cpp
cschem/svg/SymbolLibrary.cpp
cschem/circuit/SafeMap.cpp
cschem/circuit/Net.cpp
cschem/circuit/Textual.cpp
cschem/ui/HtmlDelegate.h
cschem/ui/HtmlDelegate.cpp
cschem/ui/RecentFiles.h
cschem/ui/RecentFiles.cpp
"

for a in ${=FILES}; do
    echo "target_sources(cpcb PRIVATE $a )" >> cpcb/CMakeLists.txt
done

    
