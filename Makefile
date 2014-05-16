main.exe: main.cpp huffman.cpp decoder.cpp
	g++ huffman.cpp decoder.cpp main.cpp -o main.exe -g
