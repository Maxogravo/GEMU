main:
	g++ src/GEMU.cpp -o GEMU $(shell pkg-config --cflags --libs sdl3)

tools:
	g++ src/hexconv.cpp -o hex

run:
	./GEMU

clean:
	rm -rf GEMU
	clear
