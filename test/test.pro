TEMPLATE = app
TARGET = test
INCLUDEPATH += .
INCLUDEPATH += ../cpcb	
DEFINES += QT_DEPRECATED_WARNINGS

# Input
QT += svg

SOURCES += combotest.cpp ../cpcb/ui/DimSpinner.cpp ../cpcb/ui/Expression.cpp
HEADERS += ../cpcb/ui/DimSpinner.h	

#SOURCES += unittest.cpp ../cpcb/ui/Expression.cpp

#SOURCES += cttest.cpp CTItem.cpp
#HEADERS += CTItem.h

#SOURCES += reftest.cpp
	
