all: constructed/cschem_256x256.png \
	constructed/cschem.icns \
	constructed/cschem.ico \
        constructed/cpcb_256x256.png \
	constructed/cpcb.icns \
	constructed/cpcb.ico

constructed/cschem_256x256.png \
constructed/cschem.icns \
constructed/cschem.ico: source/cschem.svg
	./makeicons.py $^

constructed/cpcb_256x256.png \
constructed/cpcb.icns \
constructed/cpcb.ico: source/cpcb.svg
	./makeicons.py $^
