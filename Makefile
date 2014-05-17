main.exe: main.cpp huffman.cpp decoder.cpp
	g++ huffman.cpp decoder.cpp main.cpp -o main.exe -Wall -Wextra -O2

.PHONY: clean
clean:
	-rm main.exe NULL *.bmp

.PHONY: test
test: main.exe
	@main gig-sn01
	@main gig-sn08
	@main monalisa
	@main teatime
