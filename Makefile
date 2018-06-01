# This makefile is for Linux and MacOS building
# Windows uses QCreator.

# Unix installation
ifdef DESTDIR
  # Debian uses this
  INSTALLPATH = $(DESTDIR)/usr
  SHAREPATH = $(DESTDIR)/usr/share
else
  INSTALLPATH = /usr/local
  SHAREPATH = /usr/local/share
endif

UNAME=$(shell uname -s)

ifeq (, $(shell which qmake-qt5))
  QMAKE=qmake
else
  QMAKE=qmake-qt5
endif

ifeq ($(UNAME),Linux)
  # Linux
  SELECTQT=QT_SELECT=5
else
  ifeq ($(UNAME),Darwin)
    # Mac OS
    QROOT=/Users/wagenaar/Qt-5.7/5.7
    QBINPATH=$(QROOT)/clang_64/bin
    QMAKE=$(QBINPATH)/qmake
  else
    $(error Unknown operating system. This Makefile is for Mac or Linux.)
  endif
endif



all: src

clean:
	+rm -rf build

src: prep
	+make -C build -f Makefile-cpcb release

debug: prep
	+make -C build -f Makefile-cpcb debug
prep:
	mkdir -p build
#	rm -f build/*/BuildDate.o
	( cd build; $(SELECTQT) $(QMAKE) ../src/cpcb.pro )

.PHONY: src  all clean tar \
        install prep debug
