#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <vector>

typedef unsigned char uint8;

class Huffman {
public:
    Huffman() { tbls.push_back(new int [256]); clear(); }
    ~Huffman() {
        for (size_t i = 0; i < tbls.size(); ++i)
            delete [] tbls[i];
    }
    void insert(int len, uint8 val);

    void clear() { _ins_tbl = _ins_idx = 0, _unit = 1<<7, _prev_len = 1; }

    std::vector<int*> tbls;
private:
    int _ins_tbl, _ins_idx, _unit, _prev_len;
};

#endif
