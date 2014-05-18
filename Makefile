main.exe: main.cpp huffman.h huffman.cpp decoder.h decoder.cpp
	g++ -Wall -Wextra -O2 huffman.cpp decoder.cpp main.cpp -o main.exe

.PHONY: clean
clean:
	-rm main.exe NULL *.bmp

.PHONY: test
test: main.exe
	@main gig-sn01 gig-sn08 monalisa teatime
