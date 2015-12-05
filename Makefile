GCC = g++
FLAGS = -std=c++11 -Wall -Wextra -Ofast -march=native

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
TARGET = main
else
TARGET = main.exe
endif


$(TARGET): main.cpp huffman.h huffman.cpp decoder.h decoder.cpp encoder.h encoder.cpp
	${GCC} ${FLAGS} huffman.cpp decoder.cpp encoder.cpp main.cpp -o main

.PHONY: clean
clean:
	-rm main main.exe *.bmp *_from_bmp.jpg

.PHONY: test
test: $(TARGET)
	@./$(TARGET) gig-sn01 gig-sn08 monalisa teatime img015 test
