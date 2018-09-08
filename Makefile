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

DOCPATH = $(SHAREPATH)/doc/cschem

# Linux and Mac building
all: release

clean:
	+rm -rf build

release: release-cpcb release-cschem

debug: debug-cpcb debug-cschem

release-cschem: prep-cschem
	+make -C build -f Makefile-cschem release

release-cpcb: prep-cpcb
	+make -C build -f Makefile-cpcb release

debug-cschem: prep-cschem
	+make -C build -f Makefile-cschem debug

debug-cpcb: prep-cpcb
	+make -C build -f Makefile-cpcb debug
prep-cschem:
	mkdir -p build
	( cd build; $(SELECTQT) $(QMAKE) ../cschem/cschem.pro )

prep-cpcb:
	mkdir -p build
	( cd build; $(SELECTQT) $(QMAKE) ../cpcb/cpcb.pro )


.PHONY: src all clean tar man install prep debug prep-cschem prep-cpcb \
	debug-cpcb debug-cschem release-cpcb debug-cschem
