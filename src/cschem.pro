# QMake project file for cschem                  -*- mode: shell-script; -*-

TEMPLATE = app
TARGET = cschem
INCLUDEPATH += .
QT += svg
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += debug_and_release
CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }
MAKEFILE = Makefile-cschem

# Input
HEADERS += ui/Scene.h   ui/SceneElement.h   ui/SceneConnection.h
SOURCES += ui/Scene.cpp ui/SceneElement.cpp ui/SceneConnection.cpp
HEADERS += ui/HoverManager.h   ui/ConnBuilder.h  
SOURCES += ui/HoverManager.cpp ui/ConnBuilder.cpp
HEADERS += circuit/Router.h   circuit/CircuitMod.h   circuit/NumberConflicts.h
SOURCES += circuit/Router.cpp circuit/CircuitMod.cpp circuit/NumberConflicts.cpp
HEADERS += ui/Style.h
SOURCES += ui/Style.cpp
HEADERS += circuit/CircuitModData.h     
SOURCES += circuit/CM_MergeSelection.cpp
HEADERS += ui/MainWindow.h   ui/Clipboard.h   ui/LibView.h
SOURCES += ui/MainWindow.cpp ui/Clipboard.cpp ui/LibView.cpp
HEADERS += ui/SceneAnnotation.h    ui/FloatingSymbol.h
SOURCES += ui/SceneAnnotation.cpp  ui/FloatingSymbol.cpp
HEADERS += ui/SceneElementData.h
HEADERS += ui/PartListView.h
SOURCES += ui/PartListView.cpp
HEADERS += svg/SvgExporter.h
SOURCES += svg/SvgExporter.cpp
SOURCES += cschem.cpp	

HEADERS += svg/Symbol.h   svg/SymbolLibrary.h   svg/XmlElement.h   svg/XmlNode.h
SOURCES += svg/Symbol.cpp svg/SymbolLibrary.cpp svg/XmlElement.cpp svg/XmlNode.cpp
HEADERS += circuit/Element.h   circuit/Circuit.h   circuit/Net.h
SOURCES += circuit/Element.cpp circuit/Circuit.cpp circuit/Net.cpp
HEADERS += circuit/PinID.h
HEADERS += circuit/IDFactory.h   circuit/Connection.h  
SOURCES += circuit/IDFactory.cpp circuit/Connection.cpp
HEADERS += circuit/Schem.h   circuit/Layer.h   file/FileIO.h
SOURCES += circuit/Schem.cpp circuit/Layer.cpp file/FileIO.cpp
HEADERS += svg/Geometry.h
SOURCES += svg/Geometry.cpp
HEADERS += circuit/PartNumbering.h
SOURCES += circuit/PartNumbering.cpp
HEADERS += ui/SignalAccumulator.h   ui/TextTable.h
SOURCES += ui/SignalAccumulator.cpp ui/TextTable.cpp
HEADERS += ui/SvgItem.h   ui/PartList.h   ui/HtmlDelegate.h
SOURCES += ui/SvgItem.cpp ui/PartList.cpp ui/HtmlDelegate.cpp
HEADERS += circuit/SafeMap.h
SOURCES += circuit/SafeMap.cpp
RESOURCES += cschem.qrc
