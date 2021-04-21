TEMPLATE = app
TARGET = cpcb
INCLUDEPATH += .
INCLUDEPATH += ../cschem
QT += widgets svg

CONFIG += debug_and_release
CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }
MAKEFILE = Makefile-cpcb
QMAKE_CXXFLAGS += -std=c++17

mac {
  ICON = cpcb.icns
  QMAKE_MAC_SDK = macosx
  QMAKE_INFO_PLIST = Info.plist
  OTHER_FILE += Info.plist
}

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += ui/MainWindow.h ui/Mode.h ui/Modebar.h  ui/Statusbar.h
SOURCES += main.cpp ui/MainWindow.cpp ui/Modebar.cpp ui/Statusbar.cpp
HEADERS += data/Dim.h   data/Layer.h   data/Point.h   data/FreeRotation.h
SOURCES +=              data/Layer.cpp data/Point.cpp data/FreeRotation.cpp
HEADERS += data/NPHole.h
SOURCES += data/NPHole.cpp
HEADERS += data/Hole.h   data/Trace.h   data/Pad.h   data/Text.h
SOURCES += data/Hole.cpp data/Trace.cpp data/Pad.cpp data/Text.cpp
HEADERS += data/Group.h  data/Object.h    data/Board.h   data/Layout.h
SOURCES += data/Group.cpp data/Object.cpp data/Board.cpp data/Layout.cpp
HEADERS += data/PCBFileIO.h   data/Rect.h   data/Arc.h   data/Segment.h
SOURCES += data/PCBFileIO.cpp data/Rect.cpp data/Arc.cpp data/Segment.cpp
HEADERS += ui/Propertiesbar.h   ui/Editor.h   ui/DimSpinner.h   ui/EData.h
SOURCES += ui/Propertiesbar.cpp ui/Editor.cpp ui/DimSpinner.cpp ui/EData.cpp
HEADERS += data/SimpleFont.h   data/Paths.h   data/LinkedSchematic.h
SOURCES += data/SimpleFont.cpp data/Paths.cpp data/LinkedSchematic.cpp
HEADERS += ui/ORenderer.h   ui/ElementView.h   ui/ComponentView.h
SOURCES += ui/ORenderer.cpp ui/ElementView.cpp ui/ComponentView.cpp
HEADERS += ui/MultiCompView.h   ui/VerticalLabel.h   ui/PinMapDialog.h
SOURCES += ui/MultiCompView.cpp ui/VerticalLabel.cpp   ui/PinMapDialog.cpp
HEADERS += data/UndoStep.h data/LayerPoint.h data/VectorCf.h
HEADERS += data/Clipboard.h   data/PCBNet.h   data/NodeID.h   data/LinkedNet.h
SOURCES += data/Clipboard.cpp data/PCBNet.cpp data/NodeID.cpp data/LinkedNet.cpp
HEADERS += data/Nodename.h   data/NetMismatch.h   data/PinMapper.h
SOURCES += data/Nodename.cpp data/NetMismatch.cpp data/PinMapper.cpp
HEADERS += gerber/Apertures.h   gerber/Font.h   gerber/Gerber.h
SOURCES += gerber/Apertures.cpp gerber/Font.cpp gerber/GerberFile.cpp
HEADERS += gerber/Collector.h   gerber/GerberFile.h gerber/GerberWriter.h
SOURCES += gerber/Collector.cpp gerber/Gerber.cpp   gerber/GerberWriter.cpp
HEADERS += gerber/PasteMaskWriter.h
SOURCES += gerber/PasteMaskWriter.cpp
HEADERS += ui/SignalNameCombo.h   ui/PinNameEditor.h   ui/Tracer.h
SOURCES += ui/SignalNameCombo.cpp ui/PinNameEditor.cpp ui/Tracer.cpp
HEADERS += data/Polyline.h   data/FilledPlane.h   data/Intersection.h
SOURCES += data/Polyline.cpp data/FilledPlane.cpp data/Intersection.cpp
HEADERS += data/TraceRepair.h   ui/PlaneEditor.h   ui/Find.h
SOURCES += data/TraceRepair.cpp ui/PlaneEditor.cpp ui/Find.cpp
HEADERS += ui/BoardSizeDialog.h   ui/Expression.h
SOURCES += ui/BoardSizeDialog.cpp ui/Expression.cpp
HEADERS += ui/Version.h   ui/BuildDate.h   gerber/FrontPanelWriter.h
SOURCES += ui/Version.cpp ui/BuildDate.cpp gerber/FrontPanelWriter.cpp
FORMS += ui/BoardSizeDialog.ui

# From CSCHEM
HEADERS += circuit/IDFactory.h
HEADERS += circuit/Schem.h
HEADERS += circuit/Circuit.h
HEADERS += circuit/Element.h
HEADERS += circuit/Connection.h
HEADERS += circuit/PartNumbering.h
HEADERS += circuit/SafeMap.h
HEADERS += circuit/Net.h
HEADERS += circuit/Textual.h
HEADERS += file/FileIO.h
HEADERS += svg/XmlElement.h
HEADERS += svg/XmlNode.h
HEADERS += svg/Symbol.h
HEADERS += svg/SymbolLibrary.h
SOURCES += ../cschem/circuit/IDFactory.cpp
SOURCES += ../cschem/circuit/Schem.cpp
SOURCES += ../cschem/circuit/Circuit.cpp
SOURCES += ../cschem/circuit/Element.cpp
SOURCES += ../cschem/circuit/Connection.cpp
SOURCES += ../cschem/circuit/PartNumbering.cpp
SOURCES += ../cschem/file/FileIO.cpp
SOURCES += ../cschem/svg/XmlElement.cpp
SOURCES += ../cschem/svg/XmlNode.cpp
SOURCES += ../cschem/svg/Symbol.cpp
SOURCES += ../cschem/svg/SymbolLibrary.cpp
SOURCES += ../cschem/circuit/SafeMap.cpp
SOURCES += ../cschem/circuit/Net.cpp
SOURCES += ../cschem/circuit/Textual.cpp

RESOURCES += ui/ui.qrc cpcb.qrc
