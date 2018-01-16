# QMake project file for cschem                  -*- mode: shell-script; -*-

HEADERS += svg/Symbol.h   svg/SymbolLibrary.h   svg/XmlElement.h   svg/XmlNode.h
SOURCES += svg/Symbol.cpp svg/SymbolLibrary.cpp svg/XmlElement.cpp svg/XmlNode.cpp
HEADERS += circuit/Element.h   circuit/Circuit.h   circuit/Net.h
SOURCES += circuit/Element.cpp circuit/Circuit.cpp circuit/Net.cpp
HEADERS += circuit/PinID.h
HEADERS += circuit/IDFactory.h   circuit/Connection.h  
SOURCES += circuit/IDFactory.cpp circuit/Connection.cpp
HEADERS += circuit/Schem.h   file/FileIO.h
SOURCES += circuit/Schem.cpp file/FileIO.cpp
HEADERS += svg/Geometry.h
SOURCES += svg/Geometry.cpp
HEADERS += circuit/PartNumbering.h
SOURCES += circuit/PartNumbering.cpp
HEADERS += qt/SignalAccumulator.h   qt/TextTable.h
SOURCES += qt/SignalAccumulator.cpp qt/TextTable.cpp
HEADERS += qt/SvgItem.h
SOURCES += qt/SvgItem.cpp
