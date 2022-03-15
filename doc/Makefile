ALL: cschem-userguide.pdf cpcb-userguide.pdf cpcb.1 cschem.1

%.1: %.1.txt
	a2x --doctype manpage --format manpage --no-xmllint --destination . $<

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

clean:; rm -f *.log *.aux cschem-userguide.pdf cpcb-userguide.pdf
