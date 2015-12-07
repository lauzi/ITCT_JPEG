#ifndef RECODER_H
#define RECODER_H

#include <string>

#include "huffman.hpp"

class Recoder {
    public:
    std::string in, out;
    Recoder (std::string i_in, std::string i_out):
        in(i_in), out(i_out) {}

    bool run();

    private:
    std::vector<int> _counts[2][2];

    void _get_counts();
    void _print_counts();
};

#endif
