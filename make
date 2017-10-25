build:
	g++ *.cpp -o dcue -std=c++11 -O3

buildnocpp11:
	g++ *.cpp -o dcue -D DISABLECPP11 -O3

clean:
	-rm -f dcue *.o

install:
	echo "currently unsupported. please move the binaries/put them into your path yourself -- sorry!"