ALL:
	mkdir -p build
	( cd build; cmake .. )
	( cd build; cmake --build . )
