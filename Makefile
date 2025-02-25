all: waterlab6

waterlab6: waterlab6.cpp
	g++ waterlab6.cpp -Wall -lX11 -lGL libggfonts.a -o lab6

clean:
	rm -f waterlab6
