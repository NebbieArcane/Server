convert: convert.cpp
	g++ convert.cpp -m32 -lgdbm -o convert
sizes: convert.cpp
	g++ sizes.cpp -m32  -o sizes32
	g++ sizes.cpp   -o sizes
all: convert sizes