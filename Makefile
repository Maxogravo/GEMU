main:
	g++ src/GEMU.cpp -o GEMU $(shell pkg-config --cflags --libs raylib)

tools:
	g++ src/hexconv.cpp -o hex

run:
	./GEMU

clean:
	rm -rf GEMU
	clear
