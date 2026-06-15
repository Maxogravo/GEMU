main:
	g++ src/GEMU.cpp -o GEMU $(shell pkg-config --cflags --libs raylib)

tools:
	g++ src/hexconv.cpp -o hex

test:
	g++ src/gtest.cpp -o gtest $(shell pkg-config --cflags --libs raylib)
	
run:
	./GEMU

clean:
	rm -rf GEMU
	clear
