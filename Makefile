all: magneto

magneto: *.cpp *.h
	g++ -fopenmp -march=native -std=c++11 -O3 helpers.cpp magneto.cpp System.cpp physics.cpp -o magneto

debug: *.cpp *.h
	g++ -fopenmp -g -std=c++11 helpers.cpp magneto.cpp System.cpp physics.cpp -o magneto

testing: *.cpp *.h
	g++ -std=c++11 helpers.cpp testing.cpp -o testing

nbstart:
	ipython notebook --profile my
