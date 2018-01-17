# QMake project file for cschem                  -*- mode: shell-script; -*-

TEMPLATE = app
TARGET = cschem
INCLUDEPATH += .
QT += svg
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += debug_and_release
CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }
MAKEFILE = Makefile-cschem

include(common.pri)

# Input
HEADERS += schemui/Scene.h   schemui/SceneElement.h   schemui/SceneConnection.h
SOURCES += schemui/Scene.cpp schemui/SceneElement.cpp schemui/SceneConnection.cpp
HEADERS += schemui/HoverManager.h   schemui/ConnBuilder.h  
SOURCES += schemui/HoverManager.cpp schemui/ConnBuilder.cpp
HEADERS += circuit/Router.h   circuit/CircuitMod.h   
SOURCES += circuit/Router.cpp circuit/CircuitMod.cpp
HEADERS += schemui/Style.h
SOURCES += schemui/Style.cpp
HEADERS += circuit/CircuitModData.h     
SOURCES += circuit/CM_MergeSelection.cpp
HEADERS += schemui/MainWindow.h   schemui/Clipboard.h   schemui/LibView.h
SOURCES += schemui/MainWindow.cpp schemui/Clipboard.cpp schemui/LibView.cpp
HEADERS += schemui/SceneAnnotation.h    schemui/FloatingSymbol.h
SOURCES += schemui/SceneAnnotation.cpp  schemui/FloatingSymbol.cpp
HEADERS += schemui/SceneElementData.h
HEADERS += schemui/PartListView.h
SOURCES += schemui/PartListView.cpp
HEADERS += svg/SvgExporter.h
SOURCES += svg/SvgExporter.cpp
SOURCES += cschem.cpp	
RESOURCES += cschem.qrc
