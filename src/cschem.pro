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
HEADERS += file/Element.h   file/Circuit.h   file/Net.h   file/PinID.h
SOURCES += file/Element.cpp file/Circuit.cpp file/Net.cpp
HEADERS += file/IDFactory.h   file/Connection.h  
SOURCES += file/IDFactory.cpp file/Connection.cpp
HEADERS += file/Schem.h   file/FileIO.h
SOURCES += file/Schem.cpp file/FileIO.cpp
HEADERS += ui/Scene.h   ui/SceneElement.h   ui/SceneConnection.h
SOURCES += ui/Scene.cpp ui/SceneElement.cpp ui/SceneConnection.cpp
HEADERS += ui/HoverManager.h   ui/ConnBuilder.h  
SOURCES += ui/HoverManager.cpp ui/ConnBuilder.cpp
HEADERS += svg/Router.h   svg/Geometry.h   svg/CircuitMod.h   ui/Style.h
SOURCES += svg/Router.cpp svg/Geometry.cpp svg/CircuitMod.cpp ui/Style.cpp
HEADERS += svg/CircuitModData.h      svg/PartNumbering.h
SOURCES += svg/CM_MergeSelection.cpp svg/PartNumbering.cpp
HEADERS += ui/MainWindow.h   ui/Clipboard.h   ui/LibView.h
SOURCES += ui/MainWindow.cpp ui/Clipboard.cpp ui/LibView.cpp
HEADERS += ui/SceneAnnotation.h   ui/SceneElementData.h
SOURCES += ui/SceneAnnotation.cpp
HEADERS += ui/PartListView.h   ui/SignalAccumulator.h   ui/TextTable.h
SOURCES += ui/PartListView.cpp ui/SignalAccumulator.cpp ui/TextTable.cpp
HEADERS += svg/Exporter.h
SOURCES += svg/Exporter.cpp
SOURCES += cschem.cpp	
RESOURCES += cschem.qrc
