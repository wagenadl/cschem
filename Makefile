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
all: release man

clean:
	+rm -rf build/
	rm -f test/*.o
	rm -f test/test

cpcb: release-cpcb
cschem: release-cschem

release: release-cpcb release-cschem

debug: debug-cpcb debug-cschem

release-cschem: prep-cschem
	+make -C build/cschem -f Makefile-cschem release

release-cpcb: prep-cpcb
	+make -C build/cpcb -f Makefile-cpcb release

debug-cschem: prep-cschem
	+make -C build/cschem -f Makefile-cschem debug

debug-cpcb: prep-cpcb
	+make -C build/cpcb -f Makefile-cpcb debug

prep-cschem:
	mkdir -p build/cschem
	rm -f build/cschem/*/BuildDate.o
	( cd build/cschem; $(SELECTQT) $(QMAKE) ../../cschem/cschem.pro )

prep-cpcb:
	mkdir -p build/cpcb
	rm -f build/cpcb/*/BuildDate.o
	( cd build/cpcb; $(SELECTQT) $(QMAKE) ../../cpcb/cpcb.pro )

install: all
	install -d $(INSTALLPATH)/bin
	install -d $(SHAREPATH)/man/man1
	install -d $(SHAREPATH)/pixmaps
	install -d $(SHAREPATH)/applications
	install -d $(SHAREPATH)/icons/gnome/128x128/mimetypes
	install -d $(SHAREPATH)/mime/packages
	install -d $(DOCPATH)
	install build/cschem/cschem $(INSTALLPATH)/bin/cschem
	install build/cpcb/cpcb $(INSTALLPATH)/bin/cpcb
	cp cschem/cschem.svg $(SHAREPATH)/pixmaps/cschem.svg
	cp cschem/cschem.png $(SHAREPATH)/pixmaps/cschem.png
	cp cschem/cschem.png $(SHAREPATH)/icons/gnome/128x128/mimetypes/cschem.png
	cp cpcb/cpcb.svg $(SHAREPATH)/pixmaps/cpcb.svg
	cp cpcb/cpcb.png $(SHAREPATH)/pixmaps/cpcb.png
	cp cpcb/cpcb.png $(SHAREPATH)/icons/gnome/128x128/mimetypes/cpcb.png
	cp cschem/cschem.xml $(SHAREPATH)/mime/packages/cschem.xml
	cp cpcb/cpcb.xml $(SHAREPATH)/mime/packages/cpcb.xml
	install cschem/cschem.desktop $(SHAREPATH)/applications/cschem.desktop
	install cpcb/cpcb.desktop $(SHAREPATH)/applications/cpcb.desktop

man: build/cpcb.1 build/cschem.1

build/%.1: doc/%.1.txt
	a2x --doctype manpage --format manpage --no-xmllint --destination build $<

userguide:;
	/bin/cp -u doc/Makefile build/Makefile.doc
	+make -C build -f Makefile.doc

tar: all
	git archive -o ../cschem.tar.gz --prefix=cschem/ HEAD

.PHONY: all clean tar man install prep debug prep-cschem prep-cpcb \
	debug-cpcb debug-cschem release-cpcb debug-cschem cschem cpcb \
	userguide
