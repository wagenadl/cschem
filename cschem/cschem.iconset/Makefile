PROG=cschem

PNGFILES=$(PROG)_8x8.png $(PROG)_16x16.png \
	$(PROG)_22x22.png $(PROG)_24x24.png \
	$(PROG)_32x32.png $(PROG)_48x48.png  \
	$(PROG)_64x64.png $(PROG)_128x128.png \
	$(PROG)_256x256.png
ICOFILE=$(PROG).ico

all:    $(PNGFILES) $(ICOFILE)	

clean:; rm -f $(PNGFILES) $(ICOFILE)

$(PROG)_%.png: ../$(PROG).svg
	inkscape --export-width  $(shell echo $*|sed 's/x.*//') \
	         --export-height $(shell echo $*|sed 's/.*x//') \
	         --export-filename $@ $<

$(PROG).ico:  $(PROG)_256x256.png
	convert -background transparent $< \
		-define icon:auto-resize=32,48,64,128 $@

.PHONY: all clean
