all: generated/cschem_256x256.png \
	generated/cschem.icns \
	generated/cschem.ico \
        generated/cpcb_256x256.png \
	generated/cpcb.icns \
	generated/cpcb.ico

generated/cschem_256x256.png \
generated/cschem.icns \
generated/cschem.ico: source/cschem.svg
	./makeicons.py $^

generated/cpcb_256x256.png \
generated/cpcb.icns \
generated/cpcb.ico: source/cpcb.svg
	./makeicons.py $^
