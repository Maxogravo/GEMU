mac_x86:
	g++ src/GEMU.cpp -o gemu $(shell pkg-config --cflags --libs raylib)

linux_x86:
	g++ src/GEMU.cpp -o gemu -lraylib -lGL -lm -lpthread -ldl

clean:
	rm -rf gemu
	clear
