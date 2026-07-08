mac_x86:
	g++ src/GEMU.cpp -o gemu $(shell pkg-config --cflags --libs raylib)


clean:
	rm -rf gemu
	clear
