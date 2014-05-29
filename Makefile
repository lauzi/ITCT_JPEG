GCC = g++
FLAGS = -std=c++11 -Wall -Wextra -Ofast -march=native

main.exe: main.cpp huffman.h huffman.cpp decoder.h decoder.cpp encoder.h encoder.cpp
	${GCC} ${FLAGS} huffman.cpp decoder.cpp encoder.cpp main.cpp -o main.exe

.PHONY: clean
clean:
	-rm main.exe NULL *.bmp *_from_bmp.jpg

.PHONY: test
test: main.exe
	@./main.exe gig-sn01 gig-sn08 monalisa teatime img015 test
