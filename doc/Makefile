ALL: cschem-userguide.pdf

%.pdf: ../doc/%.tex images
	pdflatex $<
	pdflatex $<

images: ug-battery.pdf ug-ground.pdf ug-opamp-cont.pdf ug-opamp.pdf

ug-%.pdf: ../doc/ug-%.svg
	svg2pdf $< $@

clean:; rm -f cschem-userguide.log cschem-userguide.aux cschem-userguide.pdf
