main:
	g++ src/GEMU.cpp -o GEMU

tools:
	g++ src/hexconv.cpp -o hex

run:
	./GEMU

clean:
	rm -rf GEMU
	clear
