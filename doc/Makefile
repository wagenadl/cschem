ALL: cschem-userguide.pdf

%.pdf: ../doc/%.tex images
	pdflatex $<
	pdflatex $<

images: ug-battery.pdf ug-ground.pdf ug-opamp-cont.pdf ug-opamp.pdf \
	ug-pdamp.pdf ug-pdamp-cont.pdf ug-acdc.pdf ugx-acdc-screen.png \
	ug-stereo.pdf ug-switchcont.pdf ug-switch.pdf ug-sillyswitch.pdf \
	ug-driver.pdf

ugx-%.png: ../doc/ug-%.png
	cp $< $@

ug-%.pdf: ../doc/ug-%.svg
	svg2pdf $< $@

clean:; rm -f cschem-userguide.log cschem-userguide.aux cschem-userguide.pdf
