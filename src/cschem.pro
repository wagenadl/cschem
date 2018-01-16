# QMake project file for cschem                  -*- mode: shell-script; -*-

TEMPLATE = app
TARGET = cschem
INCLUDEPATH += .
QT += svg
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += debug_and_release
CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }

# Input
HEADERS += svg/Part.h   svg/PartLibrary.h   svg/XmlElement.h   svg/XmlNode.h
SOURCES += svg/Part.cpp svg/PartLibrary.cpp svg/XmlElement.cpp svg/XmlNode.cpp
HEADERS += circuit/Element.h   circuit/Circuit.h   circuit/Net.h
SOURCES += circuit/Element.cpp circuit/Circuit.cpp circuit/Net.cpp
HEADERS += circuit/PinID.h
HEADERS += circuit/IDFactory.h   circuit/Connection.h  
SOURCES += circuit/IDFactory.cpp circuit/Connection.cpp
HEADERS += circuit/Schem.h   file/FileIO.h
SOURCES += circuit/Schem.cpp file/FileIO.cpp
HEADERS += schemui/Scene.h   schemui/SceneElement.h   schemui/SceneConnection.h
SOURCES += schemui/Scene.cpp schemui/SceneElement.cpp schemui/SceneConnection.cpp
HEADERS += schemui/HoverManager.h   schemui/ConnBuilder.h  
SOURCES += schemui/HoverManager.cpp schemui/ConnBuilder.cpp
HEADERS += circuit/Router.h   svg/Geometry.h   circuit/CircuitMod.h   
SOURCES += circuit/Router.cpp svg/Geometry.cpp circuit/CircuitMod.cpp
HEADERS += schemui/Style.h
SOURCES += schemui/Style.cpp
HEADERS += circuit/CircuitModData.h      circuit/PartNumbering.h
SOURCES += circuit/CM_MergeSelection.cpp circuit/PartNumbering.cpp
HEADERS += schemui/MainWindow.h   schemui/Clipboard.h   schemui/LibView.h
SOURCES += schemui/MainWindow.cpp schemui/Clipboard.cpp schemui/LibView.cpp
HEADERS += schemui/SceneAnnotation.h    schemui/FloatingPart.h
SOURCES += schemui/SceneAnnotation.cpp  schemui/FloatingPart.cpp
HEADERS += schemui/SceneElementData.h
HEADERS += schemui/PartListView.h   qt/SignalAccumulator.h   qt/TextTable.h
SOURCES += schemui/PartListView.cpp qt/SignalAccumulator.cpp qt/TextTable.cpp
HEADERS += svg/Exporter.h
SOURCES += svg/Exporter.cpp
HEADERS += schemui/SvgItem.h
SOURCES += schemui/SvgItem.cpp
SOURCES += cschem.cpp	
RESOURCES += cschem.qrc
