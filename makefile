all:
	g++ matrix.cpp -I src/include -L src/lib -o main -lmingw32 -lSDL2main -lSDL2
