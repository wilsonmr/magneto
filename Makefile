all: magneto

magneto: *.cpp *.h
	clang++ -fopenmp -march=native -std=c++11 -O3 helpers.cpp magneto.cpp System.cpp physics.cpp -o magneto

debug: *.cpp *.h
	clang++ -fopenmp -g -std=c++11 helpers.cpp magneto.cpp System.cpp physics.cpp -o magneto

testing: *.cpp *.h
	clang++ -std=c++11 helpers.cpp testing.cpp -o testing

nbstart:
	ipython notebook --profile my
