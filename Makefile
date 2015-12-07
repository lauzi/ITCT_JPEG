GCC = g++
FLAGS = -std=c++11 -Wall -Wextra -g #-Ofast -march=native

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
TARGET = main
else
TARGET = main.exe
endif

lib_cpps={opt_htable,huffman,decoder,encoder,recoder}.cpp

$(TARGET): main.cpp huffman.hpp huffman.cpp decoder.hpp decoder.cpp encoder.hpp encoder.cpp opt_htable.hpp opt_htable.cpp recoder.hpp recoder.cpp
	${GCC} ${FLAGS} ${lib_cpps} main.cpp -o main
#	${GCC} ${FLAGS} opt_htable.cpp huffman.cpp decoder.cpp encoder.cpp recoder.cpp main.cpp -o main

.PHONY: clean
clean:
	-rm main main.exe *.bmp *_from_bmp.jpg

.PHONY: test
test: $(TARGET)
	@./$(TARGET) gig-sn01 gig-sn08 monalisa teatime img015 test
